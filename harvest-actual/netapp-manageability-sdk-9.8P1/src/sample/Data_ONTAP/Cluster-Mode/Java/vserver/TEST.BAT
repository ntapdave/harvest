@rem This makefile assumes that your PATH contains
@rem the bin directory which has java.

@rem change the name of the filer and root account info
@rem to suit your test environment

java -classpath .;../../../../../../lib/java/classes/manageontap-9.8P1.jar;../../../../../java/classes/manageontap-9.8P1.jar vserver sweetpea root tryme

