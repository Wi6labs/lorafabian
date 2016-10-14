<?php
// the script returns all messages received by the gateway (RX).
// those messages are stored in the file received_msg.txt
// ex request http://192.168.x.x/update.php
// input parameter : none
// output parameter : all messages


// open the file
$handle = fopen('received_msg.txt', 'r');

// init the result
$allMsg="";

// if file is opened
if ($handle)
{
	// while the ead of ile is not reached
	while (!feof($handle))
	{
    // read a line
		$msg = fgets($handle);
    
    // add it in the result buffer
    $allMsg = $allMsg.$msg;
	}
  
	// close the file
	fclose($handle);
}
echo $allMsg;


?>
