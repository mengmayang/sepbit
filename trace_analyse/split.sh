#!/bin/bash

########### Modify here to set the directory of traces
ALI_DOWNLOAD_FILE_PATH="/home/yyqiao/papaer/alibaba_block_traces_2020/io_traces.csv"  # The csv trace file you downloaded from the Alibaba GitHub
ALI_TRACE_PATH="/sds_data/ali_trace/" # The directory that contains the traces split by volumes
########################################################
TRACE_PATH=$ALI_TRACE_PATH
TRACE_PREFIX="ali"
TRACE_VOLUME_PATH="etc/ali_selected.txt"
TRACE_DISPLAY_NAME="AliCloud"
SELECTED="1" # 0 for analysis of cluster iops, or 1 for volume iops

# Both volumes use the format of AliCloud
if [[ ! -d $TRACE_PATH ]]; then
  echo "TRACE_PATH not set; please set in etc/common.sh"
  exit
fi
# g++ src/get_iops.cc -o bin/get_iops -std=c++11 -O3
g++ src/select_volumes.cc -o bin/select_volumes -std=c++11 -O0 -g

################# Analysis cluster iops
if [[ $SELECTED -eq 0 ]]; then
  bin/select_volumes $ALI_DOWNLOAD_FILE_PATH $ALI_TRACE_PATH
fi
################# Analysis volume iops
if [[ $SELECTED -eq 1 ]]; then
  bin/select_volumes $ALI_DOWNLOAD_FILE_PATH $ALI_TRACE_PATH $TRACE_VOLUME_PATH
fi
