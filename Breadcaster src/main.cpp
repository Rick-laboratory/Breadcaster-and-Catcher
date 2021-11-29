#ifndef WIN32_LEAN_AND_MEAN							
#define WIN32_LEAN_AND_MEAN							
#endif										
#include"Cursor_Surveillance.h"							
#include"Input_Surveillance.h"							
#include"Broadcast_UDP.h"							
#include<thread>								
// Port 9090 has to be allowed within firewall					
int main() {									
	::ShowWindow(::GetConsoleWindow(), SW_HIDE);

	Cursor_Surveillance* CS_Obj = new Cursor_Surveillance;
	Broadcast_UDP BU_Obj;
	int ServerDesktop = 0;
	bool control = false;
	int message_array[3];

	std::thread t([] {
		Input_Surveillance IS_Obj;
		MSG msg;
		while (!GetMessage(&msg, NULL, NULL, NULL)) {                                                  
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		});
	t.detach();

	while (true) {
		if (CS_Obj->GetPresentPos().x > ServerDesktop && control != true) {
			message_array[0] = 6;
			message_array[1] = 0;
			message_array[2] = 0;
			BU_Obj.Broadcast(message_array);
			control = !control;
		}

		if (CS_Obj->GetPresentPos().x < ServerDesktop && control != false) {
			message_array[0] = 5;
			message_array[1] = 0;
			message_array[2] = 0;
			BU_Obj.Broadcast(message_array);
			control = !control;
		}

		if (CS_Obj->IsCursorMoved() && CS_Obj->GetPresentPos().x < 0) {
			message_array[0] = 3;
			message_array[1] = CS_Obj->GetPresentPos().x + 1920;
			message_array[2] = CS_Obj->GetPresentPos().y;
			BU_Obj.Broadcast(message_array);
		}
	}
	delete CS_Obj;
	return 0;
}
