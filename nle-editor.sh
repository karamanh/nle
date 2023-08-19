#!/bin/bash
cwd=$(pwd)
appname=nle-editor
appdir=output
app="$cwd/$appdir/$appname"

exec $app
exit 0