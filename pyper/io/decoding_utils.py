import struct

import pandas as pd


def fract_to_float(fract, factor):
    return (float(fract) / 32768) * factor


def intlist_to_int_4bytes(int_list):
    h = "".join(f"{i:02x}" for i in int_list)
    return struct.unpack("<I", bytes.fromhex(h))[0]


def intlist_to_int_2bytes(int_list):
    h = "".join(f"{i:02x}" for i in int_list)
    return struct.unpack("<h", bytes.fromhex(h))[0]


def intlist_to_float_4bytes(int_list):
    h = "".join(f"{i:02x}" for i in int_list)
    return struct.unpack("<f", bytes.fromhex(h))[0]


def extract_data_from_standard_frame(frame, pno_mode, timestamp=None):
    if pno_mode == "standard":
        frame_dat = frame
        start_index = 48
        n_sensors = frame[40 : 40 + 4]
        frames = intlist_to_int_4bytes(frame_dat[32 : 32 + 4])
        timestamp = timestamp
    else:
        frame_dat = frame[1]
        start_index = 28
        frames = intlist_to_int_4bytes(frame_dat[12 : 12 + 4])
        n_sensors = frame_dat[20 : 20 + 4]
        timestamp = frame[0]

    df = pd.DataFrame()
    out_list = []
    for i in range(n_sensors[0]):
        in_list = []

        in_list.append(int(i + 1))
        in_list.append(int(frames))
        in_list.append(timestamp)
        in_list.append(
            intlist_to_float_4bytes(frame_dat[start_index : start_index + 4])
        )
        in_list.append(
            intlist_to_float_4bytes(frame_dat[start_index + 4 : start_index + 8])
        )
        in_list.append(
            intlist_to_float_4bytes(frame_dat[start_index + 8 : start_index + 12])
        )
        in_list.append(
            intlist_to_float_4bytes(frame_dat[start_index + 12 : start_index + 16])
        )
        in_list.append(
            intlist_to_float_4bytes(frame_dat[start_index + 16 : start_index + 20])
        )
        in_list.append(
            intlist_to_float_4bytes(frame_dat[start_index + 20 : start_index + 24])
        )

        out_list.append(in_list)
        start_index += 32
    df = pd.DataFrame(
        out_list,
        columns=[
            "sensor_n",
            "frames",
            "time",
            "x",
            "y",
            "z",
            "azimuth",
            "elevation",
            "roll",
        ],
    )
    return df


def extract_data_from_acceleration_frame(
    frame, sampling_mode, orientation, conv_factor, timestamp=None
):
    if sampling_mode == "single":
        frame_dat = frame
        start_index = 48
        n_sensors = frame[40 : 40 + 4]
        frames = intlist_to_int_4bytes(frame_dat[32 : 32 + 4])
        timestamp = timestamp
    elif sampling_mode == "continuous":
        frame_dat = frame[1]
        start_index = 28
        frames = intlist_to_int_4bytes(frame_dat[12 : 12 + 4])
        n_sensors = frame_dat[20 : 20 + 4]
        timestamp = frame[0]

    if orientation == "euler_degrees":
        cols = [
            "sensor_n",
            "frames",
            "time",
            "x",
            "y",
            "z",
            "azimuth",
            "elevation",
            "roll",
            "x_acc",
            "y_acc",
            "z_acc",
            "xyz_acc_magnitude",
        ]
    elif orientation == "quaternion":
        cols = [
            "sensor_n",
            "frames",
            "time",
            "x",
            "y",
            "z",
            "q0",
            "q1",
            "q2",
            "q3",
            "q4",
            "x_acc",
            "y_acc",
            "z_acc",
            "xyz_acc_magnitude",
        ]

    df = pd.DataFrame()
    out_list = []
    for i in range(n_sensors[0]):
        in_list = []

        in_list.append(int(i + 1))
        in_list.append(int(frames))
        in_list.append(timestamp)
        in_list.append(
            intlist_to_float_4bytes(frame_dat[start_index : start_index + 4])
        )
        in_list.append(
            intlist_to_float_4bytes(frame_dat[start_index + 4 : start_index + 8])
        )
        in_list.append(
            intlist_to_float_4bytes(frame_dat[start_index + 8 : start_index + 12])
        )
        in_list.append(
            fract_to_float(
                intlist_to_int_2bytes(frame_dat[start_index + 12 : start_index + 14]),
                conv_factor,
            )
        )

        in_list.append(
            fract_to_float(
                intlist_to_int_2bytes(frame_dat[start_index + 14 : start_index + 16]),
                conv_factor,
            )
        )

        in_list.append(
            fract_to_float(
                intlist_to_int_2bytes(frame_dat[start_index + 16 : start_index + 18]),
                conv_factor,
            )
        )

        if orientation == "quaternion":
            in_list.append(
                fract_to_float(
                    intlist_to_int_2bytes(
                        frame_dat[start_index + 18 : start_index + 20]
                    ),
                    conv_factor,
                )
            )

        in_list.append(  # Start calculating the acceleration
            fract_to_float(
                intlist_to_int_2bytes(frame_dat[start_index + 20 : start_index + 22]),
                factor=16.0,
            )
        )
        in_list.append(
            fract_to_float(
                intlist_to_int_2bytes(frame_dat[start_index + 22 : start_index + 24]),
                factor=16.0,
            )
        )
        in_list.append(
            fract_to_float(
                intlist_to_int_2bytes(frame_dat[start_index + 24 : start_index + 26]),
                factor=16.0,
            )
        )
        in_list.append(
            fract_to_float(
                intlist_to_int_2bytes(frame_dat[start_index + 26 : start_index + 28]),
                factor=16.0,
            )
        )

        out_list.append(in_list)
        start_index += 32
    df = pd.DataFrame(
        out_list,
        columns=cols,
    )
    return df
