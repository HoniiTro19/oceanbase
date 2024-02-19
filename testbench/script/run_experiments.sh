#!/usr/bin/bash

function testbench() {
  python /home/zhd/oceanbase/testbench/script/cmd.py $@
}

function check_cmd_status() {
  MSG=$1
  if [ $? -ne 0 ]; then
    echo "Error: command $MSG failed" >&2
    exit 1
  fi
}

function check_file_exists() {
  FILE=$1
  if [ ! -f $FILE ]; then
    echo "Error: file $FILE does not exist." >&2
    exit 1
  fi
}

function check_directory_exists() {
  DIR=$1
  if [ ! -d $DIR ]; then
    echo "Error: directory $DIR does not exists." >&2
    exit 1
  fi
}

function deploy_cluster() {
  testbench cluster deploy -c $cluster_config
  check_cmd_status "deploy cluster"
  testbench bench load -c $workload_config
  check_cmd_status "load dataset"
}

function destroy_cluster() {
  testbench cluster destroy
  check_cmd_status "destroy cluster"
}

function distributed_transaction() {
  delays=(1 20 50 100)
  participants=(1 2 3)
  operations=(10 20 30 40 50 60)
  workload_config="$CONFIG/workload/distributed_transaction.yaml"
  check_file_exists $workload_config
  for delay in ${delays[@]}; do
    for participant in ${participants[@]}; do
      for operation in ${operations[@]}; do
        sed -i "s/participants:.*/participants: $participant/" $workload_config
        check_cmd_status "update config participants"
        sed -i "s/operations:.*/operations: $operation/" $workload_config
        check_cmd_status "update config operations"

        deploy_cluster

        sed -i "s/time:.*/time: 1/" $workload_config
        check_cmd_status "update config time"
        time=$(date -u +%FT%T%z)
        trace="distributedtransaction-participants$participant-operations$operation-delay$delay-$time"
        testbench bench test -c $workload_config -t "$trace-warmup"
        check_cmd_status "distributed transaction warmup"
        sed -i "s/time:.*/time: 5/" $workload_config
        check_cmd_status "update config time"
        testbench bench mocknet -c $cluster_config -d $delay -l 0
        check_cmd_status "simulate network environment"
        testbench bench test -c $workload_config -t $trace
        check_cmd_status "run distributed transaction"
        testbench bench resetnet
        check_cmd_status "reset network environment"
        testbench report histogram -d ~/.testbench/scheduler/$trace/log -f ~/linux-fonts/simsun.ttc -s 5
        check_cmd_status "analyze histogram"

        destroy_cluster
      done
    done
  done
}

function contention_transaction() {
  # delays=(1 20 50 100)
  # concurrencys=(5 10 15 20)
  # operations=(1 5 10 15 20)
  delays=(1)
  concurrencys=(5)
  operations=(1)
  workload_config="$CONFIG/workload/contention_transaction.yaml"
  check_file_exists $workload_config
  for delay in ${delays[@]}; do
    for concurrency in ${concurrencys[@]}; do
      for operation in ${operations[@]}; do
        sed -i "s/concurrency:.*/concurrency: $concurrency/" $workload_config
        check_cmd_status "update config concurrency"
        sed -i "s/operations:.*/operations: $operation/" $workload_config
        check_cmd_status "update config operations"

        # deploy_cluster

        sed -i "s/time:.*/time: 1/" $workload_config
        check_cmd_status "update config time"
        time=$(date -u +%FT%T%z)
        trace="contentiontransaction-concurrency$concurrency-operations$operation-delay$delay-$time"
        # testbench bench test -c $workload_config -t "$trace-warmup" -e
        # check_cmd_status "contention transaction warmup"
        sed -i "s/time:.*/time: 1/" $workload_config
        check_cmd_status "update config time"
        testbench bench mocknet -c $cluster_config -d $delay -l 0
        check_cmd_status "simulate network environment"
        testbench bench test -c $workload_config -t $trace -e
        check_cmd_status "run contention transaction"
        testbench bench resetnet
        check_cmd_status "reset network environment"
        testbench report histogram -d ~/.testbench/scheduler/$trace/log -f ~/linux-fonts/simsun.ttc -s 1
        check_cmd_status "analyze histogram"      

        # destroy_cluster
      done
    done
  done
}

function deadlock_transaction() {
  # delays=(1 20 50 100)
  # concurrencys=(6 12 18 24 30)
  # chains=(1 2 3)
  delays=(1)
  concurrencys=(6)
  chains=(1)
  workload_config="$CONFIG/workload/deadlock_transaction.yaml"
  check_file_exists $workload_config
  for delay in ${delays[@]}; do
    for concurrency in ${concurrencys[@]}; do
      for chain in ${chains[@]}; do
        sed -i "s/concurrency:.*/concurrency: $concurrency/" $workload_config
        check_cmd_status "update config concurrency"
        sed -i "s/chains:.*/chains: $chain/" $workload_config
        check_cmd_status "update config chains"

        # deploy_cluster

        sed -i "s/time:.*/time: 1/" $workload_config
        check_cmd_status "update config time"
        time=$(date -u +%FT%T%z)
        trace="deadlocktransaction-concurrency$concurrency-chains$chain-delay$delay-$time"
        testbench bench test -c $workload_config -t "$trace-warmup" -l
        check_cmd_status "deadlock transaction warmup"
        sed -i "s/time:.*/time: 1/" $workload_config
        check_cmd_status "update config time"
        testbench bench mocknet -c $cluster_config -d $delay -l 0
        check_cmd_status "simulate network environment"
        testbench bench test -c $workload_config -t $trace -l
        check_cmd_status "run deadlock transaction"
        testbench bench resetnet
        check_cmd_status "reset network environment"
        testbench report histogram -d ~/.testbench/scheduler/$trace/log -f ~/linux-fonts/simsun.ttc -s 1
        check_cmd_status "analyze histogram"    
      
        # destroy_cluster
      done
    done
  done
}

function concurrent_transaction() {
  # delays=(1 20 50 100)
  # operations=(0 10 20 30)
  # readonlys=(0 20 40 60 80 100)
  delays=(1)
  operations=(10)
  readonlys=(0)
  workload_config="$CONFIG/workload/concurrent_transaction.yaml"
  check_file_exists $workload_config
  for delay in ${delays[@]}; do
    for operation in ${operations[@]}; do
      for readonly in ${readonlys[@]}; do
        sed -i "s/operations:.*/operations: $operation/" $workload_config
        check_cmd_status "update config operations"
        sed -i "s/readonly:.*/readonly: $readonly/" $workload_config
        check_cmd_status "update config readonly"

        # deploy_cluster

        sed -i "s/time:.*/time: 1/" $workload_config
        check_cmd_status "update config time"
        time=$(date -u +%FT%T%z)
        trace="concurrenttransaction-operations$operation-readonly$readonly-delay$delay-$time"
        testbench bench test -c $workload_config -t "$trace-warmup"
        check_cmd_status "concurrent transaction warmup"
        sed -i "s/time:.*/time: 1/" $workload_config
        check_cmd_status "update config time"
        testbench bench mocknet -c $cluster_config -d $delay -l 0
        check_cmd_status "simulate network environment"
        testbench bench test -c $workload_config -t $trace
        check_cmd_status "run concurrent transaction"
        testbench bench resetnet
        check_cmd_status "reset network environment"
        testbench report histogram -d ~/.testbench/scheduler/$trace/log -f ~/linux-fonts/simsun.ttc -s 1
        check_cmd_status "analyze histogram"   
      
        # destroy_cluster
      done
    done
  done
}

if [ $# -le 1 ] ; then
  echo "Usage: $(basename $0) CONFIG FUNCTION" >&2
  exit 1
fi

CONFIG=`cd $1 && pwd`
check_directory_exists $CONFIG
cluster_config="$CONFIG/cluster/config.yaml"
check_file_exists $cluster_config

shift
FUNCTION=$2
if declare -f $FUNCTION > /dev/null
then 
  $@
else 
  echo "Error: $FUNCTION is not a known function name, Options: [distributed_transaction|contention_transaction|deadlock_transaction|concurrent_transaction]" >&2
  exit 1
fi