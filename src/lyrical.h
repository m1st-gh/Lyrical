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
struct discord *init_bot();
void on_interaction_create(struct discord *client, const struct discord_interaction *event);
void on_ready(struct discord *client, const struct discord_ready *event);
void play_song(struct discord *client, const struct discord_interaction *event, struct coglink_client *c_client, u64snowflake GUILD_ID);
void lyrical_stats(struct coglink_client *client, struct coglink_node *node, struct coglink_stats *stats);
void lyrical_ready(struct coglink_client *client, struct coglink_node *node, struct coglink_ready *ready);
#endif