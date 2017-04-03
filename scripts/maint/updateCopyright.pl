#!/usr/bin/perl -i -w -p

$NEWYEAR=2017;

s/Copyright(.*) (201[^7]), The Spider Project/Copyright$1 $2-${NEWYEAR}, The Spider Project/;

s/Copyright(.*)-(20..), The Spider Project/Copyright$1-${NEWYEAR}, The Spider Project/;
