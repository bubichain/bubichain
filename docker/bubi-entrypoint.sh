/docker-entrypoint.sh postgres

sleep 10

exec $BUBI_HOME/bin/bubi $*
