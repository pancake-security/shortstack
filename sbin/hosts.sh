#!/usr/bin/env bash

# Run a shell command on all servers.
#
# Environment Variables
#
#   JIFFY_SERVERS    File naming remote servers.
#     Default is ${JIFFY_CONF_DIR}/servers.
#   JIFFY_CONF_DIR  Alternate conf dir. Default is ${JIFFY_HOME}/conf.
#   JIFFY_HOST_SLEEP Seconds to sleep between spawning remote commands.
#   JIFFY_SSH_OPTS Options passed to ssh when running remote commands.
##

usage="Usage: servers.sh command..."

# if no args specified, show usage
if [ $# -le 0 ]; then
  echo $usage
  exit 1
fi

sbin="`dirname "$0"`"
sbin="`cd "$sbin"; pwd`"


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
    ssh $JIFFY_SSH_OPTS "$host" $"${@// /\\ }" \
      2>&1 | sed "s/^/$host: /"
  else
    ssh $JIFFY_SSH_OPTS "$host" $"${@// /\\ }" \
      2>&1 | sed "s/^/$host: /" &
  fi
  if [ "$JIFFY_HOST_SLEEP" != "" ]; then
    sleep $JIFFY_HOST_SLEEP
  fi
done

wait
