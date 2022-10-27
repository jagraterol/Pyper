import struct

import numpy as np


def viper_crc16(data):
    """Calculate the CRC16 and return it as a list of int

    Parameters
    ----------
    data : list of int
        The data to calculate the CRC16 for. It should contain the complete message. That means from preamble to CRC (not including the CRC itself).

    Returns
    -------
    crc16 : list of int
    """

    def _viper_crc16(data, crc):
        """Meta function to calculate the CRC16

        It uses the formula defined in the VNCP

        Parameters
        ----------
        data : list of int
            The data to calculate the CRC16 for. It should contain the complete message. That means from preamble to CRC (not including the CRC itself).
        crc : int

        Returns
        -------
        crc : int
        """
        a = np.array([0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0])
        data = (data ^ (crc)) & 0xFF
        crc >>= 8

        if a[data & 0xF] ^ a[data >> 4]:
            crc ^= 0xC001

        data <<= 6
        crc ^= data
        data <<= 1
        crc ^= data

        return crc

    crc = 0

    for i in range(len(data)):
        crc = _viper_crc16(data[i], crc)

    # Pack the CRC16 and then extract the hex values as int
    crc16 = [int(x) for x in struct.pack("<I", crc)]

    return crc16
