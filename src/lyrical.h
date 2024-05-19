#include <concord/discord.h>
#include <concord/discord_codecs.h>
#include <concord/types.h>
#include <stdlib.h>
#include <string.h>
#include <coglink/types.h>
#include <coglink/websocket.h>
#include <concord/discord-events.h>
#include <coglink/utils.h>

#ifndef LYRICAL_H
#define LYRICAL_H

/**
 * @brief Initializes the bot.
 * 
 * This function initializes the Discord bot and returns a pointer to the initialized discord structure.
 * 
 * @return A pointer to the initialized discord structure.
 */
struct discord *init_bot();

/**
 * @brief Handles the interaction create event.
 * 
 * This function is called when an interaction create event occurs.
 * It takes the discord client and the interaction event as parameters.
 * 
 * @param client The discord client.
 * @param event The interaction event.
 */
void on_interaction_create(struct discord *client, const struct discord_interaction *event);

/**
 * @brief Handles the ready event.
 * 
 * This function is called when the bot is ready to start receiving events.
 * It takes the discord client and the ready event as parameters.
 * 
 * @param client The discord client.
 * @param event The ready event.
 */
void on_ready(struct discord *client, const struct discord_ready *event);

/**
 * @brief Plays a song.
 * 
 * This function is called to play a song.
 * It takes the discord client, the interaction event, and the coglink client as parameters.
 * 
 * @param client The discord client.
 * @param event The interaction event.
 * @param c_client The coglink client.
 */
void play_song(struct discord *client, const struct discord_interaction *event, struct coglink_client *c_client);

/**
 * @brief Skips a song.
 * 
 * This function is called to skip the currently playing song.
 * It takes the discord client, the interaction event, and the coglink client as parameters.
 * 
 * @param client The discord client.
 * @param event The interaction event.
 * @param c_client The coglink client.
 */
void skip_song(struct discord *client, const struct discord_interaction *event, struct coglink_client *c_client);

/**
 * @brief Stops the bot.
 * 
 * This function is called to stop the bot.
 * It takes the discord client, the interaction event, and the coglink client as parameters.
 * 
 * @param client The discord client.
 * @param event The interaction event.
 * @param c_client The coglink client.
 */
void stop(struct discord *client, const struct discord_interaction *event, struct coglink_client *c_client);

/**
 * @brief Handles the pp event.
 * 
 * This function is called to handle the pp event.
 * It takes the discord client, the interaction event, and the coglink client as parameters.
 * 
 * @param client The discord client.
 * @param event The interaction event.
 * @param c_client The coglink client.
 */
void pp(struct discord *client, const struct discord_interaction *event, struct coglink_client *c_client);

/**
 * @brief Clears the bot's state.
 * 
 * This function is called to clear the bot's state.
 * It takes the discord client, the interaction event, and the coglink client as parameters.
 * 
 * @param client The discord client.
 * @param event The interaction event.
 * @param c_client The coglink client.
 */
void clear(struct discord *client, const struct discord_interaction *event, struct coglink_client *c_client);

/**
 * @brief Makes the bot rejoin.
 * 
 * This function is called to make the bot rejoin.
 * It takes the discord client, the interaction event, and the coglink client as parameters.
 * 
 * @param client The discord client.
 * @param event The interaction event.
 * @param c_client The coglink client.
 */
void rejoin(struct discord *client, const struct discord_interaction *event, struct coglink_client *c_client);

/**
 * @brief Retrieves the queue from the specified Discord client and interaction event.
 *
 * This function is called to retrieve the queue from the specified Discord client and interaction event.
 * It takes the discord client, the interaction event, and the coglink client as parameters.
 * 
 * @param client The Discord client.
 * @param event The interaction event.
 * @param c_client The coglink client.
 */
void get_queue(struct discord *client, const struct discord_interaction *event, struct coglink_client *c_client);

/**
 * @brief Handles the queue event.
 * 
 * This function is called to handle the queue event.
 * It takes the discord client, the interaction event, and the coglink client as parameters.
 * 
 * @param client The discord client.
 * @param event The interaction event.
 * @param c_client The coglink client.
 */
void pop_queue(struct discord *client, const struct discord_interaction *event, struct coglink_client *c_client);

void button_test(struct discord *client, const struct discord_interaction *event, struct coglink_client *c_client);
void return_test(struct discord *client, const struct discord_interaction *event, struct coglink_client *c_client);
static void done_test (struct discord * client, struct discord_response *resp, const struct discord_interaction_response *event);




/**
 * @brief Handles the lyrical stats event.
 * 
 * This function is called to handle the lyrical stats event.
 * It takes the coglink client, the coglink node, and the coglink stats as parameters.
 * 
 * @param client The coglink client.
 * @param node The coglink node.
 * @param stats The coglink stats.
 */
void lyrical_stats(struct coglink_client *client, struct coglink_node *node, struct coglink_stats *stats);

/**
 * @brief Handles the lyrical ready event.
 * 
 * This function is called to handle the lyrical ready event.
 * It takes the coglink client, the coglink node, and the coglink ready as parameters.
 * 
 * @param client The coglink client.
 * @param node The coglink node.
 * @param ready The coglink ready.
 */
void lyrical_ready(struct coglink_client *client, struct coglink_node *node, struct coglink_ready *ready);

#endif