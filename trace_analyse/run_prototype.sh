#!/bin/sh
########### Modify here to set the directory of traces
ALI_VOLUME_TRACE_FILE_DIR="/sds_data/ali_trace/volume_trace" # The volumes csv trace file
BIN_DIR="/home/yyqiao/papaer/mengmayang/sepbit/prototype/build/app"
OUTPUT_DIR="/tmp/local" # log path
placement="SepBIT"
gpt_group=("0.10" "0.15" "0.20" "0.25" "0.30")

for gpt in "${gpt_group[@]}"; do
    i=0
    find "/sds_data/ali_trace/volume_trace/" -type f -name "*.csv" -print0 | while IFS= read -r -d '' file; do
        i=$((i+1))
        traceFileName=$(basename -- "$file" .csv)
        app="$BIN_DIR/app"
        log="$OUTPUT_DIR/$traceFileName.$gpt.log"
        echo "$i: $app $placement $ALI_VOLUME_TRACE_FILE_DIR $traceFileName $gpt > $log"
        #$app $placement $ALI_VOLUME_TRACE_FILE_DIR $traceFileName $gpt > $log
        sleep 2
        #rm -rf /tmp/local/*.data
    done
done

