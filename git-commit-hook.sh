#!/bin/sh
grep -nR TODO src -R > TODO-source.txt
grep -nR TODO experiments -R >> TODO-source.txt
