#!/usr/bin/env bash

sbin="`dirname "$0"`"
sbin="`cd "$sbin"; pwd`"


config=$1

# If the servers file is specified in the command line,
# then it takes precedence over the definition in
# jiffy-env.sh. Save it here.
if [ -f "$JIFFY_SERVERS" ]; then
  SERVERLIST=`cat "$JIFFY_SERVERS"`
fi


if [ "$SERVERLIST" = "" ]; then
  if [ "$JIFFY_SERVERS" = "" ]; then
    if [ -f "zedro.hosts" ]; then
      SERVERLIST=`cat "zedro.hosts"`
    else
      echo "zedro.hosts not found"
      exit 1
    fi
  else
    SERVERLIST=`cat "${JIFFY_SERVERS}"`
  fi
fi


# By default disable strict host key checking
if [ "$JIFFY_SSH_OPTS" = "" ]; then
  JIFFY_SSH_OPTS="-o StrictHostKeyChecking=no"
fi

for host in `echo "$SERVERLIST"|sed  "s/#.*$//;/^$/d"`; do
  if [ -n "${JIFFY_SSH_FOREGROUND}" ]; then
    ssh $JIFFY_SSH_OPTS "$host" cat /local/deploy/out.txt \
      2>&1 | sed "s/^/$host: /" > ~/zedro-eval/$config-host$host.txt 
  else
    ssh $JIFFY_SSH_OPTS "$host" cat /local/deploy/out.txt \
      2>&1 | sed "s/^/$host: /" > ~/zedro-eval/$config-host$host.txt &
  fi
  if [ "$JIFFY_HOST_SLEEP" != "" ]; then
    sleep $JIFFY_HOST_SLEEP
  fi
done

wait

# sbin/hosts.sh cat /local/deploy/out.txt > ~/zedro-eval/$config.txt

$sbin/analyze.sh $config