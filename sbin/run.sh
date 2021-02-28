
# Usage: sbin/run.sh <load*100> <k> <n>
# Example: sbin/run.sh 50 8 32

sbin="`dirname "$0"`"
sbin="`cd "$sbin"; pwd`"


l=$1;
load=$(awk -v load=$l '{print load/100}');
k=$2;
n=$3; 

echo "Load"
echo $load;

$sbin/hosts.sh /local/deploy/start.sh $((128*1024)) 5000 $load $n $k; sleep 25; $sbin/hosts.sh /local/deploy/stop.sh; $sbin/collect.sh ata32-n$n-k$k-l$l