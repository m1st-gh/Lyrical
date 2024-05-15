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
 * Initializes the bot.
 * 
 * @return A pointer to the initialized discord structure.
 */
struct discord *init_bot();

/**
 * Handles the interaction create event.
 * 
 * @param client The discord client.
 * @param event The interaction event.
 */
void on_interaction_create(struct discord *client, const struct discord_interaction *event);

/**
 * Handles the ready event.
 * 
 * @param client The discord client.
 * @param event The ready event.
 */
void on_ready(struct discord *client, const struct discord_ready *event);

/**
 * Plays a song.
 * 
 * @param client The discord client.
 * @param event The interaction event.
 * @param c_client The coglink client.
 */
void play_song(struct discord *client, const struct discord_interaction *event, struct coglink_client *c_client);

/**
 * Skips a song.
 * 
 * @param client The discord client.
 * @param event The interaction event.
 * @param c_client The coglink client.
 */
void skip_song(struct discord *client, const struct discord_interaction *event, struct coglink_client *c_client);

/**
 * Stops the bot.
 * 
 * @param client The discord client.
 * @param event The interaction event.
 * @param c_client The coglink client.
 */
void stop(struct discord *client, const struct discord_interaction *event, struct coglink_client *c_client);

/**
 * Handles the pp event.
 * 
 * @param client The discord client.
 * @param event The interaction event.
 * @param c_client The coglink client.
 */
void pp(struct discord *client, const struct discord_interaction *event, struct coglink_client *c_client);

/**
 * Clears the bot's state.
 * 
 * @param client The discord client.
 * @param event The interaction event.
 * @param c_client The coglink client.
 */
void clear(struct discord *client, const struct discord_interaction *event, struct coglink_client *c_client);

/**
 * Makes the bot rejoin.
 * 
 * @param client The discord client.
 * @param event The interaction event.
 * @param c_client The coglink client.
 */
void rejoin(struct discord *client, const struct discord_interaction *event, struct coglink_client *c_client);

/**
 * Handles the queue event.
 * 
 * @param client The discord client.
 * @param event The interaction event.
 * @param c_client The coglink client.
 */
void get_queue (struct discord *client, const struct discord_interaction *event, struct coglink_client *c_client);

/**
 * Handles the queue event.
 * 
 * @param client The discord client.
 * @param event The interaction event.
 * @param c_client The coglink client.
 */
void pop_queue(struct discord *client, const struct discord_interaction *event, struct coglink_client *c_client);

/**
 * Handles the lyrical stats event.
 * 
 * @param client The coglink client.
 * @param node The coglink node.
 * @param stats The coglink stats.
 */
void lyrical_stats(struct coglink_client *client, struct coglink_node *node, struct coglink_stats *stats);

/**
 * Handles the lyrical ready event.
 * 
 * @param client The coglink client.
 * @param node The coglink node.
 * @param ready The coglink ready.
 */
void lyrical_ready(struct coglink_client *client, struct coglink_node *node, struct coglink_ready *ready);
#endif