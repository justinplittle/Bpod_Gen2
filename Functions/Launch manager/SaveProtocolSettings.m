%{
----------------------------------------------------------------------------

This file is part of the Sanworks Bpod repository
<<<<<<< HEAD
Copyright (C) 2016 Sanworks LLC, Sound Beach, New York, USA
=======
Copyright (C) 2017 Sanworks LLC, Stony Brook, New York, USA
>>>>>>> 9dd1b1005e57c9d9ff3bf3532524677d5dfa1801

----------------------------------------------------------------------------

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, version 3.

<<<<<<< HEAD
This program is distributed  WITHOUT ANY WARRANTY and without even the 
implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  
=======
This program is distributed  WITHOUT ANY WARRANTY and without even the
implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
>>>>>>> 9dd1b1005e57c9d9ff3bf3532524677d5dfa1801
See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
%}
function SaveProtocolSettings(ProtocolSettings)
global BpodSystem
save(BpodSystem.Path.Settings, 'ProtocolSettings');