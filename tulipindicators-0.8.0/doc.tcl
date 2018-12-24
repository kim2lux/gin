# Tulip Indicators
# https://tulipindicators.org/
# Copyright (c) 2010-2016 Tulip Charts LLC
# Lewis Van Winkle (LV@tulipcharts.org)
#
# This file is part of Tulip Indicators.
#
# Tulip Indicators is free software: you can redistribute it and/or modify it
# under the terms of the GNU Lesser General Public License as published by the
# Free Software Foundation, either version 3 of the License, or (at your
# option) any later version.
#
# Tulip Indicators is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
# FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License
# for more details.
#
# You should have received a copy of the GNU Lesser General Public License
# along with Tulip Indicators.  If not, see <http://www.gnu.org/licenses/>.



puts "Generating documentation"




set h [open "indicators.h" r]
set header [read $h]
set header [split $header \n]
close $h

#Returns the C prototype for a given indicator
proc get_prototype {full_name} {
    global header
    set i [lsearch -exact $header "/* $full_name */"]
    if {$i == -1} {
        puts "Couldn't find prototype for: $full_name"
        exit 1
    }

    set l $i
    while {[lindex $header $l] ne ""} {
        incr l
    }

    return [join [lrange $header $i $l-1] \n]
}


rename open open_old
proc open {n m} {
    puts "Writing $n"
    set k [open_old $n $m]
    fconfigure $k -translation {auto lf}
    return $k
}



file mkdir docs

set per 5

set indicators [exec sample --list]
set indicators [split $indicators "\n"]


#Make index of options and such for php
set php_array {}
foreach i $indicators {
    array set ind $i
    set members {}
    foreach m {type name full_name} {
        lappend members "\"$m\" => \"$ind($m)\""
    }
    foreach m {inputs options outputs} {
        if {[llength $ind($m)] == 0} {
            lappend members "\"$m\" => array()"
        } else {
            lappend members "\"$m\" => array(\"[join $ind($m) {", "}]\")"
        }
    }

    lappend php_array "\"$ind(name)\" => array([join $members ", "])"
}

set untest [open [file join tests untest.txt] w]
puts $untest "# These tests are generated by sample.c and doc.tcl"
puts $untest "# They can be ran with smoke.c"
puts $untest "# Missing a test here doesn't necessarily indicate a fault"
puts $untest "# but it means that output has changed from a previous version\n\n"


set o [open [file join docs index.php] w]
puts $o {<?php}
puts $o {}
puts $o {#GENERATED BY doc.tcl}
puts $o {#DO NOT MODIFY DIRECTLY}
puts $o {}
puts $o {$ti_indicators = array(}
puts $o [join $php_array ",\n"]
puts $o {);}
puts $o {}


puts $o {$ti_prototype = array();}
puts $o {$ti_example = array();}
puts $o {$ti_lua_example = array();}
puts $o {$ti_table = array();}
puts $o {}



foreach i $indicators {
    array set ind $i

    #puts "$ind(name) $ind(options) "

    set options {}

    foreach opt $ind(options) {
        switch $opt {
            period {lappend options 5}
            "acceleration factor step" {lappend options .2}
            "acceleration factor maximum" {lappend options 2}
            "alpha" {lappend options .2}
            "stddev" {lappend options 2}
            "short period" {lappend options 2}
            "medium period" {lappend options 3}
            "long period" {lappend options 5}
            "signal period" {lappend options 9}
            "%k period" {lappend options 5}
            "%k slowing period" {lappend options 3}
            "%d period" {lappend options 3}
            default {
                puts "\n\nERROR: Unhandled option $opt"
                exit
            }
        }
    }


    puts -nonewline $o "\$ti_prototype\['$ind(name)'\] = \""
    puts -nonewline $o [get_prototype $ind(full_name)]
    puts $o "\";\n"





    set oline {}
    foreach opt $ind(options) oval $options {
        lappend oline "$opt = $oval"
    }
    set oline [join $oline ", "]

    set cmd [list sample $ind(name) {*}$options]
    set table [exec {*}$cmd]


    puts -nonewline $o "\$ti_table\['$ind(name)'\] = \""
    puts $o $oline
    puts -nonewline $o $table
    puts $o "\";\n"



    #Need to transpose the table to create test data
    set table [regsub -all {[ ]+} $table " "]
    puts $untest "\n[join [list $ind(name) {*}$options]]"
    set table [split $table "\n"]
    set cols [lindex $table 0]
    set rest [lrange $table 1 end]
    foreach col $cols {
        if {$col eq "date"} continue
        set cnum [lsearch -exact $cols $col]
        set bars {}
        foreach l $rest {
            set v [lindex [split $l " "] $cnum]
            if {$v ne {}} {lappend bars $v}
        }
        puts $untest "{[join $bars ,]}"
    }




    #C example code
    puts -nonewline $o "\$ti_example\['$ind(name)'\] = \""
    puts $o "/* Example usage of $ind(full_name) */"

    if {$ind(inputs) == "real"} {
        puts $o "/* Assuming that 'input' is a pre-loaded array of size 'in_size'. */"
        set inputs input
    } elseif {$ind(inputs) == "real real"} {
        puts $o "/* Assuming that 'input1' and 'input2' are pre-loaded arrays of size 'in_size'. */"
        set inputs "input1 input2"
    } else {
        if {[llength $ind(inputs)] == 2} {
            set ins "'[lindex $ind(inputs) 0]' and '[lindex $ind(inputs) end]'"
        } else {
            set ins "[join '[lrange $ind(inputs) 0 end-1] "', '"]', and '[lindex $ind(inputs) end]'"
        }
        set inputs $ind(inputs)
        puts $o "/* Assuming that $ins are pre-loaded arrays of size 'in_size'. */"
    }

    if {$options == {}} {
        set oc "No options"
    } else {
        set oc [join $ind(options) ", "]
    }

    set outc [join $ind(outputs) ", "]

    puts $o "TI_REAL *inputs\[\] = {[join $inputs {, }]};"
    puts $o "TI_REAL options\[\] = {[join $options {, }]}; /* $oc */";
    puts $o "TI_REAL *outputs\[[llength $ind(outputs)]\]; /* $outc */";

    puts $o "\n/* Determine how large the output size is for our options. */"
    puts $o "const int out_size = in_size - ti_$ind(name)_start(options);"

    if {[llength $ind(outputs)] > 1} {
        puts $o "\n/* Allocate memory for each output. */"
    } else {
        puts $o "\n/* Allocate memory for output. */"
    }

    for {set i 0} {$i < [llength $ind(outputs)]} {incr i} {
        puts -nonewline $o "outputs\[$i\] = malloc(sizeof(TI_REAL) * out_size);"
        puts $o " assert(outputs\[$i\] != 0); /* [lindex $ind(outputs) $i] */"

    }

    puts $o "\n/* Run the actual calculation. */"
    puts $o "const int ret = ti_$ind(name)(in_size, inputs, options, outputs);"
    puts $o "assert(ret == TI_OKAY);"

    puts $o "\";\n"




    #Lua example code
    puts -nonewline $o "\$ti_lua_example\['$ind(name)'\] = \""
    puts $o "-- Example usage of $ind(full_name)"
    puts $o "[join $ind(outputs) {, }] = ti.$ind(name)([join [list {*}$inputs {*}$options] {, }])"
    puts $o "\";\n"


}



puts $o {?>}
close $o



foreach i $indicators {
    array set ind $i
    set name $ind(name)
    set fname $ind(full_name)
    if {![file exists [file join www pages $name.md]]} {
        puts "WARNING: no documentation for $name ($fname)"
    }
}


