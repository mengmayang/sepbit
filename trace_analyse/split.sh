#!/bin/bash

########### Modify here to set the directory of traces
ALI_DOWNLOAD_FILE_PATH="/home/yyqiao/papaer/alibaba_block_traces_2020/io_traces.csv"  # The csv trace file you downloaded from the Alibaba GitHub
ALI_TRACE_PATH="/sds_data/ali_trace/" # The directory that contains the traces split by volumes
ALI_TRACE_PATH_1="/sdt_data/ali_trace/" # The directory that contains the iops of volumes
########################################################
TRACE_PATH=$ALI_TRACE_PATH
TRACE_PREFIX="ali"
TRACE_VOLUME_PATH="etc/ali_selected.txt"
TRACE_DISPLAY_NAME="AliCloud"
TRACE_IOPS_BW_PATH="$ALI_TRACE_PATH/iops_bw"
TRACE_IOPS_BW_PATH_1="$ALI_TRACE_PATH_1/iops_bw"
SELECTED="1" # 0 for split io_trace , 1 for analysis of volume iops, or 2 for analysis of cluster iops

# Both volumes use the format of AliCloud
if [[ ! -d $TRACE_PATH ]]; then
  echo "TRACE_PATH not set; please set in etc/common.sh"
  exit
fi
#g++ src/select_volumes.cc -o bin/select_volumes -std=c++11 -O0 -g
g++ src/select_volumes.cc -o bin/select_volumes -std=c++11 -O3

################# split volume io trace
if [[ $SELECTED -eq 0 ]]; then
  echo "For split volume trace"
  echo "ALI_DOWNLOAD_FILE_PATH is $ALI_DOWNLOAD_FILE_PATH"
  echo "ALI_TRACE_PATH is $ALI_TRACE_PATH"
  echo "TRACE_VOLUME_PATH is $TRACE_VOLUME_PATH"
  bin/select_volumes $ALI_DOWNLOAD_FILE_PATH $ALI_TRACE_PATH $TRACE_VOLUME_PATH
fi

################# Analysis volume iops
if [[ $SELECTED -eq 1 ]]; then
  echo "For analysis of volume iops"
  echo "TRACE_IOPS_BW_PATH is $TRACE_IOPS_BW_PATH"
  VOLUMES_TRACE_PATH="$ALI_TRACE_PATH/volume_trace"
  # 遍历指定目录及其子目录下的所有文件
  for file in "$VOLUMES_TRACE_PATH"/*.csv
  do
    # 检查文件是否存在并确认其扩展名为.csv
    if [ -f "$file" ] && [[ $file == *.csv ]]
    then
        echo "Processing $file..."
        echo "bin/select_volumes $file $TRACE_IOPS_BW_PATH_1 1"
        bin/select_volumes $file $TRACE_IOPS_BW_PATH_1 1
    fi
  done
fi

################# Analysis cluster iops
if [[ $SELECTED -eq 2 ]]; then
  echo "For analysis of cluster iops"
  echo "ALI_DOWNLOAD_FILE_PATH is $ALI_DOWNLOAD_FILE_PATH"
  echo "TRACE_IOPS_BW_PATH is $TRACE_IOPS_BW_PATH 2"
  bin/select_volumes $ALI_DOWNLOAD_FILE_PATH $TRACE_IOPS_BW_PATH 2
fi