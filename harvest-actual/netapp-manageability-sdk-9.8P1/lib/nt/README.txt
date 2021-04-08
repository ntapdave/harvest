Libraries included with the Manage ONTAP SDK:
--------------------------------------------
	          
Note to Windows users

The following static libs must be included in every project that uses ONTAPI:

    * libadt.lib
    * libnetapp.lib
    * libxml.lib

For debug versions of your code, use the versions with "d" appended to the 
filename, e.g. libadtd.lib etc.

For Multithreaded applications, use the versions with "md" appended to the 
filename, e.g. libadtd_md.lib etc.

For Multithreaded applications in debug mode, use the versions with "mdd" 
appended to the filename, e.g. libadtd_mdd.lib etc.

If you want to use RPC as the codepath (see the documentation for 
"na_server_style" or the hello_ontapi program), you must also include the 
dynamic library:

    * ntapadmin.lib

and install the DLL ntapadmin.dll in the system32 directory of every 
workstation that will be running your program.

Please note that the respective libraries for 64 bit platform are present in 
\lib\nt\x64 and \lib\nt\ia64.

 
