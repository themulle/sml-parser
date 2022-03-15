#!/bin/bash

DIR=`dirname "$0"`

for file in $DIR/../libsml-testing/*.bin
do
    echo "Running parser for $file ..."
    cat $file | $DIR/build/parser
    echo ""
done
