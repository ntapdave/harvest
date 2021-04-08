@rem This makefile assumes that your PATH contains
@rem the bin directory which has java and javac.
@rem
@rem You may have to adjust the value of classpath to
@rem find the manageontap-9.8P1.jar file.

javac -classpath ../../../../../../lib/java/classes/manageontap-9.8P1.jar;../../../../../java/classes/manageontap-9.8P1.jar  system_mode.java

