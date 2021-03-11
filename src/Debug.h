#include "RemoteDebug.h"        //https://github.com/JoaoLopesF/RemoteDebug


// Disable all debug ?
// Important to compile for prodution/release
// Disable all debug ? Good to release builds (production)
// as nothing of RemoteDebug is compiled, zero overhead :-)
// Uncomment the line below, to do it:
//#define DEBUG_DISABLED true
// Disable Websocket? This is used with RemoteDebugApp connection
// Uncomment the line below, to do it:
//#define WEBSOCKET_DISABLED true

#ifndef WEBSOCKET_DISABLED // Only if Web socket enabled (RemoteDebugApp)
// If enabled, you can change the port here (8232 is default)
// Uncomment the line below, to do it:
//#define WEBSOCKET_PORT 8232

// Internally, the RemoteDebug uses a local copy of the arduinoWebSockets library (https://github.com/Links2004/arduinoWebSockets)
// Due it not in Arduino Library Manager
// If your project already use this library,
// Uncomment the line below, to do it:
//#define USE_LIB_WEBSOCKET true
#endif



#define SERIAL_DEBUG_DISABLED true
