#!/bin/bash


# auto setup graphite with default configs if /opt/graphite/storage is missing
# needed for the use case when a docker host volume is mounted at:
#  - /opt/graphite/storage

echo "Container is starting in Harvest-only mode"
if [[ -z $GRAPHITE_SERVER ]]; then
	echo "No graphite server specified"
	exit 1
fi


