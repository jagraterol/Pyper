# Pyper

Control the Polhemus Viper via USB with Python. This implementation is based on the Viper Native Command Protocol (VNCP). For the help files of the VNCP and C/C++ header files, please write an email to the Polhemus technical support: techsupport@polhemus.com

The documentation and library is still a work in progress.

## Installation
---------------
At the present time, this repository can be installed locally in a conda environment with build:

```
# pip install build

git clone https://github.com/jagraterol/Pyper

cd Pyper

python -m build
```
This creates a dist folder inside the repository that contains a .whl file. Then:

```
cd dist
pip install <.whl file>
```

## Note on the implementation
-----------------------------
Currently, not all the functionality is implemented. The current features are:
- Requesting single frames
- Requesting which units are currently set
- Starting the continuous sampling mode
- Requesting frames when in continuous sampling mode
- Stopping the continuous sampling mode
- Translating the obtained frames

## Examples
-----------
Create a Viper instance and connect to it to be able to send and receive messages:
```
from pyper.viper_classes import PolhemusViper

viper = PolhemusViper()
viper.connect()
```

Get the current units:
```
viper.get_units()

# The current position units are: cm
# The current orientation units are: euler_degrees
```

Get a single PNO frame. Note that the device must not be in continuous sampling mode:

```
viper.get_single_pno(pno_mode="standard")

# or for PNO frames that also contain acceleration info:
viper.get_single_pno(pno_mode="acceleration")
```

To record multiple frames, the read_continuous() method can be used. Note that this method is meant to be run in an extra thread and that the Viper device must be in continuous sampling mode.
```
from threading import Event
from multiprocessing.pool import ThreadPool

viper.start_continuous(pno_mode="acceleration", frame_counting="reset_frames") # "reset_frames" means that the first frame after starting the continuous mode will have an index == 0

stop_event = Event()
pool = ThreadPool(processes=1)
async_result = pool.apply_async(viper.read_continuous, [stop_event])

time.sleep(2)

stop_event.set()
result = async_result.get()

viper.stop_continuous()
```
The output of the previous code is a list of tuples. Each tuple contains a timestamp that represent the time when the computer running the code requested the frame, and a list of integers (the message). Thus, result[0] == (timestamp, [0, 3, 4...]).

One option to translate these data:
```
import pandas as pd

from pyper.io.decoding_utils import extract_data_from_acceleration_frame


df = pd.DataFrame()
for i in range(len(result)):

    df_frame = extract_data_from_acceleration_frame(result[i], "continuous", orientation="euler_degrees", conv_factor=viper.conf['conversion_factor']['euler_degrees'])
    df = pd.concat([df, df_frame], ignore_index=True)
```
Note that the conversion factors as defined by the VNCP are available in the conf attribute of PolhemusViper.

## TODO
-------
- Implement stylus support.
- Implement changing settings.
- Refactor decoding functions.
