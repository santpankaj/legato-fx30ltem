# This test script should be executed from the localhost/tests/bin directory

# Define LOG level before starting anything.
# If legato is already running, this will not affect it.
export LE_LOG_LEVEL=DEBUG

# Start legato system processes; returns warning if the processes are already running.
#startlegato
# Start legato and load the test config
/home/pankaj/Documents/yoctosrc_fx30LTECatM/legato-af_fx30ltem/bin/startlegato && config import /apps/testOne /home/pankaj/Documents/yoctosrc_fx30LTECatM/legato-af_fx30ltem/apps/test/avcService/assetData/asset_v2.cfg \
            && config import /lwm2m /home/pankaj/Documents/yoctosrc_fx30LTECatM/legato-af_fx30ltem/apps/test/avcService/assetData/legato.cfg

# Separate LOG level for the test exe
export LE_LOG_LEVEL=DEBUG
./testAssetData
# Run the test in pass-through mode
#./testAssetData -p
echo "Tests failed:" $?

