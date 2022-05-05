import pandas as pd
import struct
import datetime


def intlist_to_int(int_list):
    h = "".join(f"{i:02x}" for i in int_list)
    return struct.unpack("<I", bytes.fromhex(h))[0]


def intlist_to_float(int_list):
    h = "".join(f"{i:02x}" for i in int_list)
    return struct.unpack("<f", bytes.fromhex(h))[0]


def extract_data_from_frame(frame):
    start_index = 48
    frames = intlist_to_int(frame[32 : 32 + 4])
    n_sensors = frame[40 : 40 + 4]

    timestamp = datetime.datetime.now().isoformat()

    # df = pd.DataFrame()
    out_list = []
    for i in range(n_sensors[0]):
        in_list = []

        in_list.append(int(i + 1))
        in_list.append(int(frames))
        in_list.append(timestamp)
        in_list.append(intlist_to_float(frame[start_index : start_index + 4]))
        in_list.append(intlist_to_float(frame[start_index + 4 : start_index + 8]))
        in_list.append(intlist_to_float(frame[start_index + 8 : start_index + 12]))
        in_list.append(intlist_to_float(frame[start_index + 12 : start_index + 16]))
        in_list.append(intlist_to_float(frame[start_index + 16 : start_index + 20]))
        in_list.append(intlist_to_float(frame[start_index + 20 : start_index + 24]))

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
