#include "lyrical.h"
#include <asm-generic/errno.h>
#include <coglink/codecs.h>
#include <coglink/lavalink.h>
#include <coglink/rest.h>
#include <coglink/types.h>
#include <coglink/utils.h>
#include <concord/channel.h>
#include <concord/discord-events.h>
#include <concord/discord.h>
#include <concord/discord_codecs.h>
#include <concord/interaction.h>
#include <concord/log.h>
#include <concord/queue.h>
#include <concord/types.h>
#include <concord/user.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

void play_song(struct discord *client, const struct discord_interaction *event, struct coglink_client *c_client) {

    char *songName = event->data->options->array->value;
    struct coglink_player *player = coglink_create_player(c_client, event->guild_id);
    if (!player) {
        struct discord_embed embed = {
            .timestamp = discord_timestamp(client),
        };
        discord_embed_set_title(&embed, "Failed to get the node.");

        struct discord_interaction_response params = {
            .type = DISCORD_INTERACTION_CHANNEL_MESSAGE_WITH_SOURCE,
            .data =
                &(struct discord_interaction_callback_data){
                    .embeds =
                        &(struct discord_embeds){
                            .size = 1,
                            .array = &embed,
                        },
                },
        };
        discord_create_interaction_response(client, event->id, event->token, &params, NULL);
        return;
    }

    struct coglink_user *user = coglink_get_user(c_client, event->member->user->id);
    if (user == NULL) {
        struct discord_embed embed = {
            .timestamp = discord_timestamp(client),
        };
        discord_embed_set_title(&embed, "You are not in a channel...");

        struct discord_interaction_response params = {
            .type = DISCORD_INTERACTION_CHANNEL_MESSAGE_WITH_SOURCE,
            .data =
                &(struct discord_interaction_callback_data){
                    .embeds =
                        &(struct discord_embeds){
                            .size = 1,
                            .array = &embed,
                        },
                },
        };
        discord_create_interaction_response(client, event->id, event->token, &params, NULL);
        return;
    }
    coglink_join_voice_channel(c_client, client, event->guild_id, user->channel_id);
    CURL *curl = curl_easy_init();

    if (!curl) {
        struct discord_embed embed = {
            .timestamp = discord_timestamp(client),
        };
        discord_embed_set_title(&embed, "Failed to initialize cURL");

        struct discord_interaction_response params = {
            .type = DISCORD_INTERACTION_CHANNEL_MESSAGE_WITH_SOURCE,
            .data =
                &(struct discord_interaction_callback_data){
                    .embeds =
                        &(struct discord_embeds){
                            .size = 1,
                            .array = &embed,
                        },
                },
        };
        discord_create_interaction_response(client, event->id, event->token, &params, NULL);
        return;
    }
    char *search = curl_easy_escape(curl, songName, strlen(songName));

    if (!search) {
        struct discord_embed embed = {
            .timestamp = discord_timestamp(client),
        };
        discord_embed_set_title(&embed, "Failed to escape the search query");

        struct discord_interaction_response params = {
            .type = DISCORD_INTERACTION_CHANNEL_MESSAGE_WITH_SOURCE,
            .data =
                &(struct discord_interaction_callback_data){
                    .embeds =
                        &(struct discord_embeds){
                            .size = 1,
                            .array = &embed,
                        },
                },
        };

        discord_create_interaction_response(client, event->id, event->token, &params, NULL);
        return;
    }
    char *searchQuery = malloc(strlen(search) + sizeof("ytsearch:") + 1);

    if (!searchQuery) {
        struct discord_embed embed = {
            .timestamp = discord_timestamp(client),
        };
        discord_embed_set_title(&embed, "Failed to allocate memory for the search query.");

        struct discord_interaction_response params = {
            .type = DISCORD_INTERACTION_CHANNEL_MESSAGE_WITH_SOURCE,
            .data =
                &(struct discord_interaction_callback_data){
                    .embeds =
                        &(struct discord_embeds){
                            .size = 1,
                            .array = &embed,
                        },
                },
        };

        discord_create_interaction_response(client, event->id, event->token, &params, NULL);

        curl_free(search);
        curl_easy_cleanup(curl);
        return;
    }
    if (strncmp(songName, "http://", sizeof("http://") - 1) != 0 &&
        strncmp(songName, "https://", sizeof("https://") - 1) != 0) {
        snprintf(searchQuery, strlen(search) + sizeof("ytsearch:") + 1, "ytsearch:%s", search);
    } else {
        strcpy(searchQuery, search);
    }
    struct coglink_load_tracks response = {0};

    int status = coglink_load_tracks(c_client, coglink_get_player_node(c_client, player), searchQuery, &response);
    curl_free(search);
    curl_easy_cleanup(curl);
    free(searchQuery);

    if (status == COGLINK_FAILED) {
        struct discord_embed embed = {
            .timestamp = discord_timestamp(client),
        };
        discord_embed_set_title(&embed, "Failed to load the track.");

        struct discord_interaction_response params = {
            .type = DISCORD_INTERACTION_CHANNEL_MESSAGE_WITH_SOURCE,
            .data =
                &(struct discord_interaction_callback_data){
                    .embeds =
                        &(struct discord_embeds){
                            .size = 1,
                            .array = &embed,
                        },
                },
        };
        discord_create_interaction_response(client, event->id, event->token, &params, NULL);

        return;
    }

    switch (response.type) {
    case COGLINK_LOAD_TYPE_TRACK: {
        struct coglink_load_tracks_track *track_response = response.data;

        char description[4000 + 1];
        char author[256 + 1];
        struct coglink_player_queue *queue = coglink_get_player_queue(c_client, player);
        struct discord_embed embed = {
            .timestamp = discord_timestamp(client),
        };
        if (queue->size == 0) {
            discord_embed_set_title(&embed, "Now Playing...");
            snprintf(description, sizeof(description), "`%s.`", track_response->info->title);
            struct coglink_update_player_params params = {
                .track =
                    &(struct coglink_update_player_track_params){
                        .encoded = track_response->encoded,
                    },
            };
            coglink_update_player(c_client, player, &params, NULL);
        } else {
            discord_embed_set_title(&embed, "Queued...");
            snprintf(description, sizeof(description), "`%s.`", track_response->info->title);
        }
        snprintf(author, sizeof(author), "By: %s", track_response->info->author);
        discord_embed_set_footer(&embed, author,
                                 "https://cdn.discordapp.com/avatars/1186478232875311265/"
                                 "90c3d15ae122604a197e34af31628449?size=1024",
                                 NULL);
        discord_embed_set_description(&embed, description);
        discord_embed_set_image(&embed, track_response->info->artworkUrl, NULL, 0, 0);
        discord_embed_set_footer(&embed, track_response->info->author,
                                 "https://cdn.discordapp.com/avatars/1186478232875311265/"
                                 "90c3d15ae122604a197e34af31628449?size=1024",
                                 NULL);
        struct discord_interaction_response params = {
            .type = DISCORD_INTERACTION_CHANNEL_MESSAGE_WITH_SOURCE,
            .data =
                &(struct discord_interaction_callback_data){
                    .embeds =
                        &(struct discord_embeds){
                            .size = 1,
                            .array = &embed,
                        },
                },
        };
        discord_create_interaction_response(client, event->id, event->token, &params, NULL);
        coglink_add_track_to_queue(c_client, player, track_response->encoded);
        discord_embed_cleanup(&embed);
        break;
    }
    case COGLINK_LOAD_TYPE_PLAYLIST: {
        struct coglink_load_tracks_playlist *data = response.data;

        char description[4000 + 1];
        char author[256 + 1];
        char tracks[64 + 1];

        struct coglink_player_queue *queue = coglink_get_player_queue(c_client, player);

        if (queue->size == 0) {
            snprintf(description, sizeof(description), "`%s`", data->tracks->array[0]->info->title);
            snprintf(tracks, sizeof(tracks), "Playing a playlist with %" PRIu64 " tracks...", data->tracks->size);
            snprintf(author, sizeof(author), "By: %s", data->tracks->array[0]->info->author);

            struct coglink_update_player_params params = {
                .track =
                    &(struct coglink_update_player_track_params){
                        .encoded = data->tracks->array[queue->size]->encoded,
                    },
            };

            coglink_update_player(c_client, player, &params, NULL);
        } else {
            snprintf(tracks, sizeof(tracks), "Queued %" PRIu64 " tracks...", data->tracks->size);
        }

        struct discord_embed embed = {
            .timestamp = discord_timestamp(client),
        };

        discord_embed_set_title(&embed, tracks);
        discord_embed_set_description(&embed, description);
        discord_embed_set_image(&embed, data->tracks->array[0]->info->artworkUrl, NULL, 0, 0);
        discord_embed_set_footer(&embed, author,
                                 "https://cdn.discordapp.com/avatars/1186478232875311265/"
                                 "90c3d15ae122604a197e34af31628449?size=1024",
                                 NULL);
        struct discord_interaction_response params = {
            .type = DISCORD_INTERACTION_CHANNEL_MESSAGE_WITH_SOURCE,
            .data =
                &(struct discord_interaction_callback_data){
                    .embeds =
                        &(struct discord_embeds){
                            .size = 1,
                            .array = &embed,
                        },
                },
        };
        discord_create_interaction_response(client, event->id, event->token, &params, NULL);

        for (size_t i = 0; i < data->tracks->size; i++) {
            coglink_add_track_to_queue(c_client, player, data->tracks->array[i]->encoded);
        }
        discord_embed_cleanup(&embed);
        break;
    }
    case COGLINK_LOAD_TYPE_SEARCH: {
        struct coglink_load_tracks_search *search_response = response.data;

        char description[4000 + 1];
        char author[256 + 1];
        struct coglink_player_queue *queue = coglink_get_player_queue(c_client, player);
        struct discord_embed embed = {
            .timestamp = discord_timestamp(client),
        };
        if (queue->size == 0) {
            discord_embed_set_title(&embed, "Now Playing...");
            snprintf(description, sizeof(description), "`%s.`", search_response->array[0]->info->title);
            struct coglink_update_player_params params = {.track =
                                                              &(struct coglink_update_player_track_params){
                                                                  .encoded = search_response->array[0]->encoded,
                                                              },
                                                          .volume = 100};

            coglink_update_player(c_client, player, &params, NULL);
        } else {
            discord_embed_set_title(&embed, "Queued...");
            snprintf(description, sizeof(description), "`%s.`", search_response->array[0]->info->title);
        }
        snprintf(author, sizeof(author), "By: %s", search_response->array[0]->info->author);
        discord_embed_set_footer(&embed, author,
                                 "https://cdn.discordapp.com/avatars/1186478232875311265/"
                                 "90c3d15ae122604a197e34af31628449?size=1024",
                                 NULL);
        discord_embed_set_description(&embed, description);
        discord_embed_set_image(&embed, search_response->array[0]->info->artworkUrl, NULL, 0, 0);
        struct discord_interaction_response params = {
            .type = DISCORD_INTERACTION_CHANNEL_MESSAGE_WITH_SOURCE,
            .data =
                &(struct discord_interaction_callback_data){
                    .embeds =
                        &(struct discord_embeds){
                            .size = 1,
                            .array = &embed,
                        },
                },
        };
        discord_create_interaction_response(client, event->id, event->token, &params, NULL);

        coglink_add_track_to_queue(c_client, player, search_response->array[0]->encoded);
        discord_embed_cleanup(&embed);
        break;
    }
    case COGLINK_LOAD_TYPE_EMPTY: {
        struct discord_embed embed = {
            .timestamp = discord_timestamp(client),
        };
        discord_embed_set_title(&embed, "No tracks found.");
        struct discord_interaction_response params = {
            .type = DISCORD_INTERACTION_CHANNEL_MESSAGE_WITH_SOURCE,
            .data =
                &(struct discord_interaction_callback_data){
                    .embeds =
                        &(struct discord_embeds){
                            .size = 1,
                            .array = &embed,
                        },
                },
        };
        discord_create_interaction_response(client, event->id, event->token, &params, NULL);
        discord_embed_cleanup(&embed);
        break;
    }
    case COGLINK_LOAD_TYPE_ERROR: {
        struct coglink_load_tracks_error *data = response.data;

        char title[4000 + 1];
        snprintf(title, sizeof(title), "Failed to load. %s", data->message);

        struct discord_embed embed = {
            .timestamp = discord_timestamp(client),
        };
        discord_embed_set_title(&embed, title);

        struct discord_interaction_response params = {
            .type = DISCORD_INTERACTION_CHANNEL_MESSAGE_WITH_SOURCE,
            .data =
                &(struct discord_interaction_callback_data){
                    .embeds =
                        &(struct discord_embeds){
                            .size = 1,
                            .array = &embed,
                        },
                },
        };
        discord_create_interaction_response(client, event->id, event->token, &params, NULL);
        discord_embed_cleanup(&embed);
        break;
    }
    }
    coglink_free_load_tracks(&response);
}
void skip_song(struct discord *client, const struct discord_interaction *event, struct coglink_client *c_client) {

    struct coglink_player *player = coglink_create_player(c_client, event->guild_id);

    if (!player) {
        struct discord_embed embed = {
            .timestamp = discord_timestamp(client),
        };
        discord_embed_set_title(&embed, "Failed to get the node.");

        struct discord_interaction_response params = {
            .type = DISCORD_INTERACTION_CHANNEL_MESSAGE_WITH_SOURCE,
            .data =
                &(struct discord_interaction_callback_data){
                    .embeds =
                        &(struct discord_embeds){
                            .size = 1,
                            .array = &embed,
                        },
                },
        };
        discord_create_interaction_response(client, event->id, event->token, &params, NULL);
        return;
    }
    struct coglink_player_queue *queue = coglink_get_player_queue(c_client, player);

    if (queue->size == 0) {
        printf("Queue Size: %zu\n", queue->size);
        struct discord_embed embed = {
            .timestamp = discord_timestamp(client),
        };
        discord_embed_set_title(&embed, "No tracks in the queue...");

        struct discord_interaction_response params = {
            .type = DISCORD_INTERACTION_CHANNEL_MESSAGE_WITH_SOURCE,
            .data =
                &(struct discord_interaction_callback_data){
                    .embeds =
                        &(struct discord_embeds){
                            .size = 1,
                            .array = &embed,
                        },
                },
        };
        discord_create_interaction_response(client, event->id, event->token, &params, NULL);
        return;
    }
    if (queue->size == 1) {
        printf("before size: %zu\n", queue->size);
        struct discord_embed embed = {
            .timestamp = discord_timestamp(client),
        };
        discord_embed_set_title(&embed, "No tracks to skip...");

        struct discord_interaction_response params = {
            .type = DISCORD_INTERACTION_CHANNEL_MESSAGE_WITH_SOURCE,
            .data =
                &(struct discord_interaction_callback_data){
                    .embeds =
                        &(struct discord_embeds){
                            .size = 1,
                            .array = &embed,
                        },
                },
        };
        discord_create_interaction_response(client, event->id, event->token, &params, NULL);
        return;
    }
    printf("Queue Size P: %zu\n", queue->size);
    coglink_update_player(c_client, player, &(struct coglink_update_player_params){
        .track = &(struct coglink_update_player_track_params){
            .encoded = queue->array[1]}}, NULL);
    coglink_remove_track_from_queue(c_client, player, 0);
    printf("Queue Size A: %zu\n", queue->size);
}