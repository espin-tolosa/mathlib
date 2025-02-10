# Introduction

The testing tools within the math library are designed to generate plot data outputs. These outputs must comply with the specifications outlined herein.

The primary objective is to decouple the data intended for plotting from any specific library used to render the plots.

This interface is structured in a straightforward, human-readable format, enabling users to interpret the data without requiring graphical rendering. Plot tools should implement this interface.

# Specification

## Function Ranges

 function [xmin : dx : xmax]
==========================================================
[x0, x1 ] -> [y0, y1] Y_TYPE
[x0, x1 ] -> [y0, y1] Y_TYPE
[x0, x1 ] -> [y0, y1] Y_TYPE
