<?php
// the script delete the file received_msg.txt which contains the messages received by the gateway (RX).
// ex request http://192.168.x.x/clear.php
// input parameter : none
// output parameter : none
unlink('received_msg.txt');
?>
