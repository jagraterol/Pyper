import usb
import array
import struct
import datetime
import json
import os

from ..io.viper_crc16 import viper_crc16
from ..io.io_utils import extract_data_from_frame


class PolhemusViper:
    """
    Polhemus Viper class to control a Polhemus Viper Device
    """

    polhemus_usb_vid = 0xF44  # Polhemus USB Vendor ID
    viper_usb_pid = 0xBF01  # Polhemus USB Product ID
    # [86, 80, 82, 67]  Preamble for commands "VPRC"
    viper_command_preamble = (
        b"\x56\x50\x52\x43"  # [86, 80, 82, 67]  Preamble for commands "VPRC"
    )
    viper_pno_preamble = b"\x56\x50\x52\x50"  # [86, 80, 82, 80] "VPRP"
    class_path = os.path.dirname(os.path.realpath(__file__))

    with open(os.path.join(class_path, "config.json"), "r") as f:
        conf = json.load(f)

    def __init__(self, seuid=0) -> None:
        self.seuid = seuid

    def _construct_message(
        self, viper_command, cmd_action, arg1=None, arg2=None, payload=None
    ):
        """
        Construct a general message to send to the Polhemus Viper. This function should not be called directly.
        """
        viper_cmd = self.conf["cmd_viper"][viper_command].get("cmd_number")
        cmd_action = self.conf["cmd_actions"][cmd_action]
        arg1 = self.conf["cmd_viper"][viper_command].get("arg1", {}).get(arg1, 0)
        arg2 = self.conf["cmd_viper"][viper_command].get("arg2", {}).get(arg2, 0)
        crc_size = 0
        if payload is None:
            cmd_frame = [self.seuid, viper_cmd, cmd_action, arg1, arg2, crc_size]
        else:
            payload = self.conf["cmd_viper"][viper_command]["payload"][payload]
            cmd_frame = [
                self.seuid,
                viper_cmd,
                cmd_action,
                arg1,
                arg2,
                payload,
                crc_size,
            ]
        cmd_packed = struct.pack(f"<{len(cmd_frame)}I", *cmd_frame)
        cmd_size = len(cmd_packed)  # Calculate the msg length
        cmd_size_bytes = struct.pack(f"<I", cmd_size)

        command_sequence = (
            self.viper_command_preamble + cmd_size_bytes + cmd_packed[:-4]
        )  # Remove the crc from the end
        # Create int list to calculate crc16
        command_sequence_list = [int(x) for x in command_sequence]

        crc16 = viper_crc16(command_sequence_list)  # Calculate crc16
        crc16_bytes = struct.pack(f"<{len(crc16)}B", *crc16)
        crc16_list = [int(x) for x in crc16_bytes]
        msg = array.array("B", command_sequence_list + crc16_list)
        return msg

    def _write_and_read(self, msg):
        """
        Write a message to the Polhemus Viper and read from the endpoint. This function should not be called directly.
        """
        self.dev.write(0x02, msg, 200)
        resp = self.dev.read(self.endpoint, self.conf["max_size"], 200)
        resp_list = resp.tolist()
        resp_crc16 = viper_crc16(resp_list[:-4])  # Remove the crc from the end
        resp_crc16_bytes = struct.pack(f"<{len(resp_crc16)}B", *resp_crc16)
        if resp_crc16_bytes != bytes(resp_list[-4:]):
            print("CRC16 error")
            return None
        else:
            return resp_list

    def connect(self):
        """
        Connect to the Polhemus device using PyUSB
        """
        dev = usb.core.find(
            idVendor=self.polhemus_usb_vid, idProduct=self.viper_usb_pid
        )
        if dev is None:
            raise ValueError("Device not found")

        dev.set_configuration()
        self.dev = dev
        self.endpoint = self.dev[0][(0, 0)][0]

    def get_single_pno(self, pno_mode="standard"):
        """
        Function that reads a single PNO from Polhemus Viper and prints results as df

        Parameters
        --------
        pno_mode : str
            Type of PNO frame to return. Accepts "standard" and "accelerated". "accelerated" also returns the acceleration of the sensors in addition to the position and orientation. Default is "standard"

        Returns
        --------
        df: pd.DataFrame
        """
        msg = self._construct_message(
            viper_command="cmd_single_pno", cmd_action="cmd_action_get", arg2=pno_mode
        )
        resp = self._write_and_read(msg)

        if resp is None:
            pass
        else:
            df = extract_data_from_frame(resp)
            return df

    def start_continuous(self, pno_mode="standard", frame_counting="reset_frames"):
        msg = self._construct_message(
            viper_command="cmd_continuous_pno",
            cmd_action="cmd_action_set",
            arg2=pno_mode,
            payload=frame_counting,
        )

        resp = self._write_and_read(msg)
        if resp is None:
            pass
        else:
            print(resp)

    def read_continuous(self, dev, ep, stop_condition):
        output_list = []
        output_dict = dict()
        max_size = 4 + 4 + 8 * 16 + 4

        while not stop_condition.is_set():
            resp = dev.read(ep, max_size, 200)
            timestamp = datetime.datetime.now().isoformat()
            resp_list = resp.tolist()
            resp_crc16 = viper_crc16(resp_list[:-4])  # Remove the crc from the end
            resp_crc16_bytes = struct.pack(f"<{len(resp_crc16)}B", *resp_crc16)

            if resp_crc16_bytes != bytes(resp_list[-4:]):
                print("CRC16 error")
                del resp
            else:
                output_list.append((timestamp, resp_list))
                output_dict[timestamp] = resp_list
        return output_list
        # print(resp_list)

    def stop_continuous(self):
        msg = self._construct_message(viper_command="cmd_continuous_pno", cmd_action="cmd_action_reset")
        resp = self._write_and_read(msg)
        if resp is None:
            pass
        else:
            print(resp)
        