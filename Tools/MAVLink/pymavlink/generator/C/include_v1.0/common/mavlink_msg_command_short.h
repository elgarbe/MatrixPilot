// MESSAGE COMMAND_SHORT PACKING

#define MAVLINK_MSG_ID_COMMAND_SHORT 75

typedef struct __mavlink_command_short_t
{
 float param1; ///< Parameter 1, as defined by MAV_CMD enum.
 float param2; ///< Parameter 2, as defined by MAV_CMD enum.
 float param3; ///< Parameter 3, as defined by MAV_CMD enum.
 float param4; ///< Parameter 4, as defined by MAV_CMD enum.
 uint8_t target_system; ///< System which should execute the command
 uint8_t target_component; ///< Component which should execute the command, 0 for all components
 uint8_t command; ///< Command ID, as defined by MAV_CMD enum.
 uint8_t confirmation; ///< 0: First transmission of this command. 1-255: Confirmation transmissions (e.g. for kill command)
} mavlink_command_short_t;

#define MAVLINK_MSG_ID_COMMAND_SHORT_LEN 20
#define MAVLINK_MSG_ID_75_LEN 20



#define MAVLINK_MESSAGE_INFO_COMMAND_SHORT { \
	"COMMAND_SHORT", \
	8, \
	{  { "param1", NULL, MAVLINK_TYPE_FLOAT, 0, 0, offsetof(mavlink_command_short_t, param1) }, \
         { "param2", NULL, MAVLINK_TYPE_FLOAT, 0, 4, offsetof(mavlink_command_short_t, param2) }, \
         { "param3", NULL, MAVLINK_TYPE_FLOAT, 0, 8, offsetof(mavlink_command_short_t, param3) }, \
         { "param4", NULL, MAVLINK_TYPE_FLOAT, 0, 12, offsetof(mavlink_command_short_t, param4) }, \
         { "target_system", NULL, MAVLINK_TYPE_UINT8_T, 0, 16, offsetof(mavlink_command_short_t, target_system) }, \
         { "target_component", NULL, MAVLINK_TYPE_UINT8_T, 0, 17, offsetof(mavlink_command_short_t, target_component) }, \
         { "command", NULL, MAVLINK_TYPE_UINT8_T, 0, 18, offsetof(mavlink_command_short_t, command) }, \
         { "confirmation", NULL, MAVLINK_TYPE_UINT8_T, 0, 19, offsetof(mavlink_command_short_t, confirmation) }, \
         } \
}


/**
 * @brief Pack a command_short message
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param msg The MAVLink message to compress the data into
 *
 * @param target_system System which should execute the command
 * @param target_component Component which should execute the command, 0 for all components
 * @param command Command ID, as defined by MAV_CMD enum.
 * @param confirmation 0: First transmission of this command. 1-255: Confirmation transmissions (e.g. for kill command)
 * @param param1 Parameter 1, as defined by MAV_CMD enum.
 * @param param2 Parameter 2, as defined by MAV_CMD enum.
 * @param param3 Parameter 3, as defined by MAV_CMD enum.
 * @param param4 Parameter 4, as defined by MAV_CMD enum.
 * @return length of the message in bytes (excluding serial stream start sign)
 */
static inline uint16_t mavlink_msg_command_short_pack(uint8_t system_id, uint8_t component_id, mavlink_message_t* msg,
						       uint8_t target_system, uint8_t target_component, uint8_t command, uint8_t confirmation, float param1, float param2, float param3, float param4)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
	char buf[20];
	_mav_put_float(buf, 0, param1);
	_mav_put_float(buf, 4, param2);
	_mav_put_float(buf, 8, param3);
	_mav_put_float(buf, 12, param4);
	_mav_put_uint8_t(buf, 16, target_system);
	_mav_put_uint8_t(buf, 17, target_component);
	_mav_put_uint8_t(buf, 18, command);
	_mav_put_uint8_t(buf, 19, confirmation);

        memcpy(_MAV_PAYLOAD(msg), buf, 20);
#else
	mavlink_command_short_t packet;
	packet.param1 = param1;
	packet.param2 = param2;
	packet.param3 = param3;
	packet.param4 = param4;
	packet.target_system = target_system;
	packet.target_component = target_component;
	packet.command = command;
	packet.confirmation = confirmation;

        memcpy(_MAV_PAYLOAD(msg), &packet, 20);
#endif

	msg->msgid = MAVLINK_MSG_ID_COMMAND_SHORT;
	return mavlink_finalize_message(msg, system_id, component_id, 20, 160);
}

/**
 * @brief Pack a command_short message on a channel
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param chan The MAVLink channel this message was sent over
 * @param msg The MAVLink message to compress the data into
 * @param target_system System which should execute the command
 * @param target_component Component which should execute the command, 0 for all components
 * @param command Command ID, as defined by MAV_CMD enum.
 * @param confirmation 0: First transmission of this command. 1-255: Confirmation transmissions (e.g. for kill command)
 * @param param1 Parameter 1, as defined by MAV_CMD enum.
 * @param param2 Parameter 2, as defined by MAV_CMD enum.
 * @param param3 Parameter 3, as defined by MAV_CMD enum.
 * @param param4 Parameter 4, as defined by MAV_CMD enum.
 * @return length of the message in bytes (excluding serial stream start sign)
 */
static inline uint16_t mavlink_msg_command_short_pack_chan(uint8_t system_id, uint8_t component_id, uint8_t chan,
							   mavlink_message_t* msg,
						           uint8_t target_system,uint8_t target_component,uint8_t command,uint8_t confirmation,float param1,float param2,float param3,float param4)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
	char buf[20];
	_mav_put_float(buf, 0, param1);
	_mav_put_float(buf, 4, param2);
	_mav_put_float(buf, 8, param3);
	_mav_put_float(buf, 12, param4);
	_mav_put_uint8_t(buf, 16, target_system);
	_mav_put_uint8_t(buf, 17, target_component);
	_mav_put_uint8_t(buf, 18, command);
	_mav_put_uint8_t(buf, 19, confirmation);

        memcpy(_MAV_PAYLOAD(msg), buf, 20);
#else
	mavlink_command_short_t packet;
	packet.param1 = param1;
	packet.param2 = param2;
	packet.param3 = param3;
	packet.param4 = param4;
	packet.target_system = target_system;
	packet.target_component = target_component;
	packet.command = command;
	packet.confirmation = confirmation;

        memcpy(_MAV_PAYLOAD(msg), &packet, 20);
#endif

	msg->msgid = MAVLINK_MSG_ID_COMMAND_SHORT;
	return mavlink_finalize_message_chan(msg, system_id, component_id, chan, 20, 160);
}

/**
 * @brief Encode a command_short struct into a message
 *
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param msg The MAVLink message to compress the data into
 * @param command_short C-struct to read the message contents from
 */
static inline uint16_t mavlink_msg_command_short_encode(uint8_t system_id, uint8_t component_id, mavlink_message_t* msg, const mavlink_command_short_t* command_short)
{
	return mavlink_msg_command_short_pack(system_id, component_id, msg, command_short->target_system, command_short->target_component, command_short->command, command_short->confirmation, command_short->param1, command_short->param2, command_short->param3, command_short->param4);
}

/**
 * @brief Send a command_short message
 * @param chan MAVLink channel to send the message
 *
 * @param target_system System which should execute the command
 * @param target_component Component which should execute the command, 0 for all components
 * @param command Command ID, as defined by MAV_CMD enum.
 * @param confirmation 0: First transmission of this command. 1-255: Confirmation transmissions (e.g. for kill command)
 * @param param1 Parameter 1, as defined by MAV_CMD enum.
 * @param param2 Parameter 2, as defined by MAV_CMD enum.
 * @param param3 Parameter 3, as defined by MAV_CMD enum.
 * @param param4 Parameter 4, as defined by MAV_CMD enum.
 */
#ifdef MAVLINK_USE_CONVENIENCE_FUNCTIONS

static inline void mavlink_msg_command_short_send(mavlink_channel_t chan, uint8_t target_system, uint8_t target_component, uint8_t command, uint8_t confirmation, float param1, float param2, float param3, float param4)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
	char buf[20];
	_mav_put_float(buf, 0, param1);
	_mav_put_float(buf, 4, param2);
	_mav_put_float(buf, 8, param3);
	_mav_put_float(buf, 12, param4);
	_mav_put_uint8_t(buf, 16, target_system);
	_mav_put_uint8_t(buf, 17, target_component);
	_mav_put_uint8_t(buf, 18, command);
	_mav_put_uint8_t(buf, 19, confirmation);

	_mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_COMMAND_SHORT, buf, 20, 160);
#else
	mavlink_command_short_t packet;
	packet.param1 = param1;
	packet.param2 = param2;
	packet.param3 = param3;
	packet.param4 = param4;
	packet.target_system = target_system;
	packet.target_component = target_component;
	packet.command = command;
	packet.confirmation = confirmation;

	_mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_COMMAND_SHORT, (const char *)&packet, 20, 160);
#endif
}

#endif

// MESSAGE COMMAND_SHORT UNPACKING


/**
 * @brief Get field target_system from command_short message
 *
 * @return System which should execute the command
 */
static inline uint8_t mavlink_msg_command_short_get_target_system(const mavlink_message_t* msg)
{
	return _MAV_RETURN_uint8_t(msg,  16);
}

/**
 * @brief Get field target_component from command_short message
 *
 * @return Component which should execute the command, 0 for all components
 */
static inline uint8_t mavlink_msg_command_short_get_target_component(const mavlink_message_t* msg)
{
	return _MAV_RETURN_uint8_t(msg,  17);
}

/**
 * @brief Get field command from command_short message
 *
 * @return Command ID, as defined by MAV_CMD enum.
 */
static inline uint8_t mavlink_msg_command_short_get_command(const mavlink_message_t* msg)
{
	return _MAV_RETURN_uint8_t(msg,  18);
}

/**
 * @brief Get field confirmation from command_short message
 *
 * @return 0: First transmission of this command. 1-255: Confirmation transmissions (e.g. for kill command)
 */
static inline uint8_t mavlink_msg_command_short_get_confirmation(const mavlink_message_t* msg)
{
	return _MAV_RETURN_uint8_t(msg,  19);
}

/**
 * @brief Get field param1 from command_short message
 *
 * @return Parameter 1, as defined by MAV_CMD enum.
 */
static inline float mavlink_msg_command_short_get_param1(const mavlink_message_t* msg)
{
	return _MAV_RETURN_float(msg,  0);
}

/**
 * @brief Get field param2 from command_short message
 *
 * @return Parameter 2, as defined by MAV_CMD enum.
 */
static inline float mavlink_msg_command_short_get_param2(const mavlink_message_t* msg)
{
	return _MAV_RETURN_float(msg,  4);
}

/**
 * @brief Get field param3 from command_short message
 *
 * @return Parameter 3, as defined by MAV_CMD enum.
 */
static inline float mavlink_msg_command_short_get_param3(const mavlink_message_t* msg)
{
	return _MAV_RETURN_float(msg,  8);
}

/**
 * @brief Get field param4 from command_short message
 *
 * @return Parameter 4, as defined by MAV_CMD enum.
 */
static inline float mavlink_msg_command_short_get_param4(const mavlink_message_t* msg)
{
	return _MAV_RETURN_float(msg,  12);
}

/**
 * @brief Decode a command_short message into a struct
 *
 * @param msg The message to decode
 * @param command_short C-struct to decode the message contents into
 */
static inline void mavlink_msg_command_short_decode(const mavlink_message_t* msg, mavlink_command_short_t* command_short)
{
#if MAVLINK_NEED_BYTE_SWAP
	command_short->param1 = mavlink_msg_command_short_get_param1(msg);
	command_short->param2 = mavlink_msg_command_short_get_param2(msg);
	command_short->param3 = mavlink_msg_command_short_get_param3(msg);
	command_short->param4 = mavlink_msg_command_short_get_param4(msg);
	command_short->target_system = mavlink_msg_command_short_get_target_system(msg);
	command_short->target_component = mavlink_msg_command_short_get_target_component(msg);
	command_short->command = mavlink_msg_command_short_get_command(msg);
	command_short->confirmation = mavlink_msg_command_short_get_confirmation(msg);
#else
	memcpy(command_short, _MAV_PAYLOAD(msg), 20);
#endif
}