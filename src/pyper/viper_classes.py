import os
import array
import struct
import datetime
import json

import usb

from .io.viper_crc16 import viper_crc16
from .io import decoding_utils


class PolhemusViper:
    """Class for controlling the Polhemus Viper"""

    polhemus_usb_vid = 0xF44  # Polhemus USB Vendor ID
    viper_usb_pid = 0xBF01  # Polhemus USB Product ID

    viper_command_preamble = (
        b"\x56\x50\x52\x43"  # [86, 80, 82, 67]  Preamble for commands "VPRC"
    )
    viper_pno_preamble = b"\x56\x50\x52\x50"  # [86, 80, 82, 80] "VPRP"

    # Load the JSON file with the commands
    class_path = os.path.dirname(os.path.realpath(__file__))
    with open(os.path.join(class_path, "config.json"), "r") as f:
        conf = json.load(f)

    def __init__(self, seuid=0) -> None:
        self.seuid = seuid

    def _construct_message(
        self, viper_command, cmd_action, arg1=None, arg2=None, payload=None
    ):
        """Construct a message to communicate with the Polhemus Viper. Should not be called directly.

        The VNCP defines two types of frames: command frames and position and orientation frames (PNO). All frames start with with a 4-byte PREAMBLE and SIZE field, and end with a 4-BYTE CRC-16 field. The body of the frame is comprised either by the command or the PNO frame. The byte order is LITTLE ENDIAN.

        This method work as follows:
        It creates a list of integers with the PREAMBLE + BODY + CRC of 0. Using struct.pack a byte object is then returned. The SIZE of this object is calculated and turned into a byte object. Then the byte objects for the PREAMBLE, SIZE, and BODY (removing the CRC) are concatenated. This sequence is turned into a list of int from which the CRC is calculated. The CRC16 is turned into a list of int and then an bytes array of the sequence + CRC16 is created as the final message.
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

    def _write_and_read(self, msg, continuous=False):
        """Write a message to the Polhemus Viper and read the response from the endpoint. This function should not be called directly.

        Parameters
        ----------
        msg : array.array
            Output of the _construct_message() method

        Returns
        -------
        resp_list : list of int
        timestamp : datetime.datetime

        or if the CRC16 check fails
        None
        None
        """
        if not continuous:
            self.dev.write(0x02, msg, 200)
        resp = self.dev.read(self.endpoint, self.conf["max_size"], 200)
        timestamp = datetime.datetime.now().isoformat()
        resp_list = resp.tolist()
        resp_crc16 = viper_crc16(resp_list[:-4])  # Remove the crc from the end
        resp_crc16_bytes = struct.pack(f"<{len(resp_crc16)}B", *resp_crc16)

        if resp_crc16_bytes != bytes(resp_list[-4:]):
            print("CRC16 error")
            return None, None
        else:
            return resp_list, timestamp

    def connect(self):
        """Connect to the Polhemus device using PyUSB"""
        dev = usb.core.find(
            idVendor=self.polhemus_usb_vid, idProduct=self.viper_usb_pid
        )
        if dev is None:
            raise ValueError("Device not found")

        dev.set_configuration()
        self.dev = dev
        self.endpoint = self.dev[0][(0, 0)][0]

    def get_single_pno(self, pno_mode="standard", orientation="euler_degrees"):
        """Read a single PNO from the Polhemus Viper.

        Parameters
        --------
        pno_mode : str
            Type of PNO frame to return. Accepts "standard" and "acceleration". "acceleration" also returns the acceleration of the sensors in addition to the position and orientation. Default is "standard"

        Returns
        --------
        df: pd.DataFrame
        """
        msg = self._construct_message(
            viper_command="cmd_single_pno", cmd_action="cmd_action_get", arg2=pno_mode
        )
        resp, timestamp = self._write_and_read(msg)
        conv_factor = self.conf["conversion_factor"][orientation]
        if resp is None:
            pass
        else:
            if pno_mode == "standard":
                df = decoding_utils.extract_data_from_standard_frame(
                    resp, pno_mode=pno_mode, timestamp=timestamp
                )
            elif pno_mode == "acceleration":
                df = decoding_utils.extract_data_from_acceleration_frame(
                    resp,
                    sampling_mode="single",
                    orientation=orientation,
                    conv_factor=conv_factor,
                    timestamp=timestamp,
                )
            return df

    def start_continuous(self, pno_mode="standard", frame_counting="reset_frames"):
        """Set the Polhemus Viper in continuous mode.

        Parameters
        ----------
        pno_mode : str
            Type of PNO frame to return. Accepts "standard" and "acceleration". "acceleration" also returns the acceleration of the sensors in addition to the position and orientation. Default is "standard"
        frame_counting : str
            Set the beginning frame number when starting continuous mode. Possible values are 'reset_frames' and 'continuous_frames'. If 'reset_frames' the first frame after starting the continuous sampling mode will have a value of 0, otherwise it will keep the sample number until that point. Default is'reset_frames'.
        """
        msg = self._construct_message(
            viper_command="cmd_continuous_pno",
            cmd_action="cmd_action_set",
            arg2=pno_mode,
            payload=frame_counting,
        )

        resp, _ = self._write_and_read(msg)
        if resp is None:
            pass
        else:
            print(resp)

    def read_continuous(self, stop_condition):
        """Read the frames of a Polhemus Viper that is set in continuous sampling mode.

        This method is meant to be run on a separate thread.

        Parameters
        ----------
        stop_condition : threading.Event()
            the Event object to be set when one wishes the to stop the thread.

        Returns
        -------
        output_list : list of tup
            The tuples are structured as (timestamp, list of int) where the timestamp is the time when the computer requested the frame and the list of int represents the frame message.

        """
        output_list = []

        while not stop_condition.is_set():
            resp, timestamp = self._write_and_read(msg=None, continuous=True)

            if resp is None:
                pass
            else:
                output_list.append((timestamp, resp))
        return output_list

    def stop_continuous(self):
        """Stop the continuous sampling mode"""
        msg = self._construct_message(
            viper_command="cmd_continuous_pno", cmd_action="cmd_action_reset"
        )
        resp, _ = self._write_and_read(msg)
        if resp is None:
            pass
        else:
            print(resp)
            print("Continuous streaming stopped")

    def get_units(self):
        """Prints the current position and orientation units"""
        msg = self._construct_message(
            viper_command="cmd_units", cmd_action="cmd_action_get"
        )
        resp, _ = self._write_and_read(msg)
        if resp is None:
            pass
        else:
            position = [
                x
                for x in self.conf["viper_position_units"].items()
                if decoding_utils.intlist_to_int_4bytes(resp[-12 : -12 + 4]) in x
            ]  # Get position
            orientation = [
                x
                for x in self.conf["viper_orientation_units"].items()
                if decoding_utils.intlist_to_int_4bytes(resp[-12 + 4 : -12 + 8]) in x
            ]  # Get orientation
            print(
                f"The current position units are: {position[0][0]}\nThe current orientation units are: {orientation[0][0]}"
            )

    def set_stylus_mode(self, stylus_mode):
        """Set the stylus function mode.

        Parameters
        --------
        stylus_mode : str
            The stylus mode to set. Accepts "mark", "point", "line", "toggle".
        """

        try:
            msg = self._construct_message(
                viper_command="cmd_stylus_mode",
                cmd_action="cmd_action_set",
                payload=stylus_mode,
            )
        except KeyError:
            print("Invalid stylus mode. Only accepts 'mark', 'point', 'line', 'toggle'")
            return

        resp, _ = self._write_and_read(msg)

        if resp is None:
            pass
        elif resp[16] == 3:
            print(f"Stylus mode set to: {stylus_mode}")
        elif resp[16] == 4:
            print(f"Command not acknowledged")
