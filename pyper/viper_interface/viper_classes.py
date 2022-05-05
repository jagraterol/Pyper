import usb
import numpy as np
import array
import struct

from ..io.viper_crc16 import viper_crc16
from ..io.io_utils import extract_data_from_frame


class PolhemusViper:
    polhemus_usb_vid = 0xF44
    viper_usb_pid = 0xBF01
    # [86, 80, 82, 67]  Preamble for commands "VPRC"
    viper_command_preamble = b"\x56\x50\x52\x43"
    viper_pno_preamble = b"\x56\x50\x52\x50"  # [86, 80, 82, 80] "VPRP"
    last_msg = None

    cmd_actions = {
        "cmd_action_set": 0,
        "cmd_action_get": 1,
        "cmd_action_reset": 2,
        "cmd_action_ack": 3,
        "cmd_action_nak": 4,
        "cmd_action_nak_warning": 5,
    }

    cmd_viper = {
        "cmd_hemisphere": 0,
        "cmd_fitler": 1,
        "cmd_tip_offset": 2,
        "cmd_increment": 3,
        "cmd_boresight": 4,
        "cmd_sensor_whoami": 5,
        "cmd_framerate": 6,
        "cmd_units": 7,
        "cmd_src_rotation": 8,
        "cmd_sync_mode": 9,
        "cmd_station_map": 10,
        "cmd_stylus": 11,
        "cmd_seuid": 12,
        "cmd_dual_output": 13,
        "cmd_serial_config": 14,
        "cmd_block_cfg": 15,
        "cmd_frame_count": 16,
        "cmd_bit": 17,
        "cmd_single_pno": 18,
        "cmd_continuous_pno": 19,
        "cmd_whoami": 20,
        "cmd_initialize": 21,
        "cmd_persist": 22,
        "cmd_enable_map": 23,
        "cmd_ftt_mode": 24,
        "cmd_map_status": 25,
        "cmd_sensor_blockcfg": 26,
        "cmd_source_cfg": 27,
        "cmd_predfilter_cfg": 28,
        "cmd_predfilter_ext": 29,
        "cmd_src_select": 30,
        "cmd_sns_origin": 31,
        "cmd_sns_virtual": 32,
        "cmd_src_whoami": 33,
    }  # For a command reference refer to ViperInterface.h

    pno_mode = {"standard": 0, "accelerated": 1}

    def connect(self, return_endpoint=False):

        dev = usb.core.find(
            idVendor=self.polhemus_usb_vid, idProduct=self.viper_usb_pid
        )
        if dev is None:
            raise ValueError("Device not found")

        dev.set_configuration()

        if return_endpoint == True:
            ep = dev[0][(0, 0)][0]
            return dev, ep

        return dev

    def get_single_pno(self, dev, ep, pno_mode="standard"):
        max_size = 4 + 4 + 8 * 16 + 4  # Max number of sensors
        preamble = self.viper_command_preamble  # In bytes
        seuid = 0  # We just have one Viper device
        cmd = self.cmd_viper["cmd_single_pno"]
        cmd_action = self.cmd_actions[
            "cmd_action_get"
        ]  # Only action available for single pno
        arg1 = 0  # not used for single pno
        pno_mode = self.pno_mode[pno_mode]
        crc_size = 4  # Standard crc size

        cmd_frame = [seuid, cmd, cmd_action, arg1, pno_mode, crc_size]
        cmd_packed = struct.pack(f"<{len(cmd_frame)}I", *cmd_frame)
        cmd_size = len(cmd_packed)  # Calculate the msg length
        cmd_size_bytes = struct.pack(f"<I", cmd_size)

        command_sequence = (
            preamble + cmd_size_bytes + cmd_packed[:-4]
        )  # Remove the crc from the end

        # Create int list to calculate crc16
        command_sequence_list = [int(x) for x in command_sequence]
        crc16 = viper_crc16(command_sequence_list)  # Calculate crc16
        crc16_bytes = struct.pack(f"<{len(crc16)}B", *crc16)
        crc16_list = [int(x) for x in crc16_bytes]
        msg = array.array("B", command_sequence_list + crc16_list)

        # Communicate with the device
        dev.write(0x02, msg, 200)

        resp = dev.read(ep, max_size, 200)
        resp_list = resp.tolist()
        resp_crc16 = viper_crc16(resp_list[:-4])  # Remove the crc from the end
        resp_crc16_bytes = struct.pack(f"<{len(resp_crc16)}B", *resp_crc16)

        if resp_crc16_bytes != bytes(resp_list[-4:]):
            print("CRC16 error")
        else:
            df = extract_data_from_frame(resp)
        return df


    def start_continuous()