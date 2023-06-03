idf_component_register(SRCS 
	"main.c" 
	"wifiInit.c" 
	"timeMgmt.c" 
	"mqtt.c"
	"counting.c"
	"commands.c"
	"showRoomState.c"
	"log_buffer.c"
	"smtp.c"
    EMBED_TXTFILES server_root_cert.pem INCLUDE_DIRS ".")
