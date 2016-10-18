<?php
// the script write in the file msg2send.txt, the message which should be sent (TX) by the gateway.
// ex request http://192.168.x.x/send.php?command=LoraFabian
// input parameter : command
// output parameter : none

// open the file in write mode
$monfichier = fopen('msg2send.txt', 'w');

// write the command in the file
fputs($monfichier, $_GET["command"]); 

// close the file
fclose($monfichier);
?>
