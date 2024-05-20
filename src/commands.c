#include <asm-generic/errno.h>
#include <coglink/codecs.h>
#include <coglink/lavalink.h>
#include <coglink/rest.h>
#include <coglink/types.h>
#include <coglink/utils.h>
#include <concord/channel.h>
#include <concord/discord-events.h>
#include <concord/discord-response.h>
#include <concord/discord.h>
#include <concord/discord_codecs.h>
#include <concord/error.h>
#include <concord/interaction.h>
#include <concord/log.h>
#include <concord/queue.h>
#include <concord/types.h>
#include <concord/user.h>
#include <concord/websockets.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "lyrical.h"

bool is_paused = false;
bool playing = false;
extern struct discord *CLIENT;
const struct discord_interaction *embed_event;

struct discord_component buttons[] = {
    {
        .type = DISCORD_COMPONENT_BUTTON,
        .style = DISCORD_BUTTON_SECONDARY,
        .custom_id = "pause_play_button",
        .emoji =
            &(struct discord_emoji){
                .name = "⏯",
            },
    },
    {
        .type = DISCORD_COMPONENT_BUTTON,
        .style = DISCORD_BUTTON_SECONDARY,
        .custom_id = "skip_button",
        .emoji =
            &(struct discord_emoji){
                .name = "⏭",
            },
    },
    {
        .type = DISCORD_COMPONENT_BUTTON,
        .style = DISCORD_BUTTON_SECONDARY,
        .custom_id = "stop_button",
        .emoji =
            &(struct discord_emoji){
                .name = "⏹",
            },
    },
    {
        .type = DISCORD_COMPONENT_BUTTON,
        .style = DISCORD_BUTTON_SECONDARY,
        .custom_id = "queue_button",
        .emoji =
            &(struct discord_emoji){
                .name = "📜",
            },
    },

};
struct discord_component action_rows = {
    .type = DISCORD_COMPONENT_ACTION_ROW,
    .components =
        &(struct discord_components){
            .array = buttons,
            .size = 4,
        },
};

void play_song(struct discord *client, const struct discord_interaction *event, struct coglink_client *c_client) {
    char *songName = event->data->options->array->value;
    struct coglink_player *player =
        coglink_create_player(c_client, event->guild_id);
    if (!player) {
        struct discord_embed embed = {
            .timestamp = discord_timestamp(client),
            .title = "Failed to get the node.",
        };
        struct discord_interaction_response params = {
            .type = DISCORD_INTERACTION_CHANNEL_MESSAGE_WITH_SOURCE,
            .data =
                &(struct discord_interaction_callback_data){
                    .flags = DISCORD_MESSAGE_EPHEMERAL,
                    .embeds =
                        &(struct discord_embeds){
                            .size = 1,
                            .array = &embed,
                        },
                },
        };
        discord_create_interaction_response(client, event->id, event->token,
                                            &params, NULL);
        return;
    }

    struct coglink_user *user =
        coglink_get_user(c_client, event->member->user->id);
    if (user == NULL) {
        struct discord_embed embed = {
            .timestamp = discord_timestamp(client),
            .title = "You are not in a channel...",
        };
        struct discord_interaction_response params = {
            .type = DISCORD_INTERACTION_CHANNEL_MESSAGE_WITH_SOURCE,
            .data =
                &(struct discord_interaction_callback_data){
                    .flags = DISCORD_MESSAGE_EPHEMERAL,
                    .embeds =
                        &(struct discord_embeds){
                            .size = 1,
                            .array = &embed,
                        },
                },
        };
        discord_create_interaction_response(client, event->id, event->token,
                                            &params, NULL);
        return;
    }
    coglink_join_voice_channel(c_client, client, event->guild_id,
                               user->channel_id);
    CURL *curl = curl_easy_init();

    if (!curl) {
        struct discord_embed embed = {
            .timestamp = discord_timestamp(client),
            .title = "Failed to initialize cURL.",
        };
        struct discord_interaction_response params = {
            .type = DISCORD_INTERACTION_CHANNEL_MESSAGE_WITH_SOURCE,
            .data =
                &(struct discord_interaction_callback_data){
                    .flags = DISCORD_MESSAGE_EPHEMERAL,

                    .embeds =
                        &(struct discord_embeds){
                            .size = 1,
                            .array = &embed,
                        },
                },
        };
        discord_create_interaction_response(client, event->id, event->token,
                                            &params, NULL);
        return;
    }
    char *search = curl_easy_escape(curl, songName, strlen(songName));

    if (!search) {
        struct discord_embed embed = {
            .timestamp = discord_timestamp(client),
            .title = "Failed to escape the search query.",
        };

        struct discord_interaction_response params = {
            .type = DISCORD_INTERACTION_CHANNEL_MESSAGE_WITH_SOURCE,
            .data =
                &(struct discord_interaction_callback_data){
                    .flags = DISCORD_MESSAGE_EPHEMERAL,
                    .embeds =
                        &(struct discord_embeds){
                            .size = 1,
                            .array = &embed,
                        },
                },
        };

        discord_create_interaction_response(client, event->id, event->token,
                                            &params, NULL);
        return;
    }
    char *searchQuery = malloc(strlen(search) + sizeof("ytsearch:") + 1);

    if (!searchQuery) {
        struct discord_embed embed = {
            .timestamp = discord_timestamp(client),
            .title = "Failed to allocate memory for the search query.",
        };

        struct discord_interaction_response params = {
            .type = DISCORD_INTERACTION_CHANNEL_MESSAGE_WITH_SOURCE,
            .data =
                &(struct discord_interaction_callback_data){
                    .flags = DISCORD_MESSAGE_EPHEMERAL,
                    .embeds =
                        &(struct discord_embeds){
                            .size = 1,
                            .array = &embed,
                        },
                },
        };

        discord_create_interaction_response(client, event->id, event->token,
                                            &params, NULL);

        curl_free(search);
        curl_easy_cleanup(curl);
        return;
    }
    if (strncmp(songName, "http://", sizeof("http://") - 1) != 0 &&
        strncmp(songName, "https://", sizeof("https://") - 1) != 0) {
        snprintf(searchQuery, strlen(search) + sizeof("ytsearch:") + 1,
                 "ytsearch:%s", search);
    } else {
        strcpy(searchQuery, search);
    }
    struct coglink_load_tracks response = {0};

    int status =
        coglink_load_tracks(c_client, coglink_get_player_node(c_client, player),
                            searchQuery, &response);
    curl_free(search);
    curl_easy_cleanup(curl);
    free(searchQuery);

    if (status == COGLINK_FAILED) {
        struct discord_embed embed = {
            .timestamp = discord_timestamp(client),
            .title = "Failed to load tracks.",
        };
        struct discord_interaction_response params = {
            .type = DISCORD_INTERACTION_CHANNEL_MESSAGE_WITH_SOURCE,
            .data =
                &(struct discord_interaction_callback_data){
                    .flags = DISCORD_MESSAGE_EPHEMERAL,
                    .embeds =
                        &(struct discord_embeds){
                            .size = 1,
                            .array = &embed,
                        },
                },
        };
        discord_create_interaction_response(client, event->id, event->token,
                                            &params, NULL);
        return;
    }

    switch (response.type) {
    case COGLINK_LOAD_TYPE_TRACK: {
        struct coglink_load_tracks_track *track_response = response.data;
        char title[64 + 1];
        char description[4000 + 1];
        char author[64 + 1];
        struct coglink_player_queue *queue =
            coglink_get_player_queue(c_client, player);
        snprintf(author, sizeof(author), "By: %s", track_response->info->author);
        if (queue->size == 0) {
            snprintf(description, sizeof(description), "`%s.`",
                     track_response->info->title);
            snprintf(title, sizeof(title), "Now Playing...");
            struct coglink_update_player_params params = {
                .paused = COGLINK_PAUSED_STATE_FALSE,
                .track =
                    &(struct coglink_update_player_track_params){
                        .encoded = track_response->encoded,
                    },
            };
            is_paused = false;
            embed_event = discord_claim(client, event);
            coglink_update_player(c_client, player, &params, NULL);
        } else {
            snprintf(title, sizeof(title), "Queued...");
            snprintf(description, sizeof(description), "`%s.`",
                     track_response->info->title);
            struct discord_embed embed = {
                .title = title,
                .description = description,

            };
            struct discord_interaction_response params = {
                .type = DISCORD_INTERACTION_CHANNEL_MESSAGE_WITH_SOURCE,
                .data =
                    &(struct discord_interaction_callback_data){
                        .flags = DISCORD_MESSAGE_EPHEMERAL,
                        .embeds =
                            &(struct discord_embeds){
                                .size = 1,
                                .array = &embed,
                            },
                    },
            };
            discord_create_interaction_response(client, event->id, event->token,
                                                &params, NULL);
            coglink_add_track_to_queue(c_client, player, track_response->encoded);
            break;
        }
        struct discord_embed embed = {
            .title = title,
            .description = description,
            .image =
                &(struct discord_embed_image){
                    .url = track_response->info->artworkUrl,
                },
            .footer =
                &(struct discord_embed_footer){
                    .text = author,
                    .icon_url = LYRICAL_URL,
                },
            .timestamp = discord_timestamp(client),
        };
        struct discord_interaction_response params = {
            .type = DISCORD_INTERACTION_CHANNEL_MESSAGE_WITH_SOURCE,
            .data = &(struct discord_interaction_callback_data){
                .embeds =
                    &(struct discord_embeds){
                        .size = 1,
                        .array = &embed,
                    },
                .components =
                    &(struct discord_components){
                        .size = 1,
                        .array = &action_rows,
                    },
            }};

        discord_create_interaction_response(client, embed_event->id,
                                            embed_event->token, &params, NULL);
        coglink_add_track_to_queue(c_client, player, track_response->encoded);
        break;
    }
    case COGLINK_LOAD_TYPE_PLAYLIST: {
        struct coglink_load_tracks_playlist *data = response.data;

        char title[64 + 1];
        char description[4000 + 1];
        char author[64 + 1];

        struct coglink_player_queue *queue =
            coglink_get_player_queue(c_client, player);

        if (queue->size == 0) {
            snprintf(title, sizeof(title), "Now Playing...");
            snprintf(description, sizeof(description),
                     "A playlist with %" PRIu64 " tracks, first up `%s.`",
                     data->tracks->size, data->tracks->array[0]->info->title);
            snprintf(author, sizeof(author), "By: %s",
                     data->tracks->array[0]->info->author);
            struct coglink_update_player_params params = {
                .paused = COGLINK_PAUSED_STATE_FALSE,
                .track =
                    &(struct coglink_update_player_track_params){
                        .encoded = data->tracks->array[queue->size]->encoded,
                    },
            };
            is_paused = false;
            embed_event = discord_claim(client, event);
            coglink_update_player(c_client, player, &params, NULL);
        } else {
            snprintf(title, sizeof(title), "Queued...");
            snprintf(description, sizeof(description), "%" PRIu64 " tracks...",
                     data->tracks->size);
            struct discord_embed embed = {
                .title = title,
                .description = description,

            };
            struct discord_interaction_response params = {
                .type = DISCORD_INTERACTION_CHANNEL_MESSAGE_WITH_SOURCE,
                .data =
                    &(struct discord_interaction_callback_data){
                        .flags = DISCORD_MESSAGE_EPHEMERAL,
                        .embeds =
                            &(struct discord_embeds){
                                .size = 1,
                                .array = &embed,
                            },
                    },
            };
            discord_create_interaction_response(client, event->id, event->token,
                                                &params, NULL);
            for (size_t i = 0; i < data->tracks->size; i++) {
                coglink_add_track_to_queue(c_client, player,
                                           data->tracks->array[i]->encoded);
            }
            break;
        }

        struct discord_embed embed = {
            .title = title,
            .description = description,
            .image =
                &(struct discord_embed_image){
                    .url = data->tracks->array[0]->info->artworkUrl,
                },
            .footer =
                &(struct discord_embed_footer){
                    .text = author,
                    .icon_url = LYRICAL_URL,
                },
            .timestamp = discord_timestamp(client),
        };
        struct discord_interaction_response params = {
            .type = DISCORD_INTERACTION_CHANNEL_MESSAGE_WITH_SOURCE,
            .data =
                &(struct discord_interaction_callback_data){
                    .embeds =
                        &(struct discord_embeds){
                            .size = 1,
                            .array = &embed,
                        },
                    .components =
                        &(struct discord_components){
                            .size = 1,
                            .array = &action_rows,
                        },
                },
        };
        discord_create_interaction_response(client, event->id, event->token,
                                            &params, NULL);

        for (size_t i = 0; i < data->tracks->size; i++) {
            coglink_add_track_to_queue(c_client, player,
                                       data->tracks->array[i]->encoded);
        }
        break;
    }
    case COGLINK_LOAD_TYPE_SEARCH: {
        struct coglink_load_tracks_search *search_response = response.data;
        char title[64 + 1];
        char description[4000 + 1];
        char author[64 + 1];
        struct coglink_player_queue *queue =
            coglink_get_player_queue(c_client, player);
        if (queue->size == 0) {
            snprintf(description, sizeof(description), "`%s.`",
                     search_response->array[0]->info->title);
            snprintf(title, sizeof(title), "Now Playing...");
            snprintf(author, sizeof(author), "By: %s",
                     search_response->array[0]->info->author);
            struct coglink_update_player_params params = {
                .paused = COGLINK_PAUSED_STATE_FALSE,
                .track =
                    &(struct coglink_update_player_track_params){
                        .encoded = search_response->array[0]->encoded,
                    },
            };
            is_paused = false;
            embed_event = discord_claim(client, event);
            coglink_update_player(c_client, player, &params, NULL);
        } else {
            snprintf(title, sizeof(title), "Queued...");
            snprintf(description, sizeof(description), "`%s.`",
                     search_response->array[0]->info->title);
            struct discord_embed embed = {
                .title = title,
                .description = description,

            };
            struct discord_interaction_response params = {
                .type = DISCORD_INTERACTION_CHANNEL_MESSAGE_WITH_SOURCE,
                .data =
                    &(struct discord_interaction_callback_data){
                        .flags = DISCORD_MESSAGE_EPHEMERAL,
                        .embeds =
                            &(struct discord_embeds){
                                .size = 1,
                                .array = &embed,
                            },
                    },
            };
            discord_create_interaction_response(client, event->id, event->token,
                                                &params, NULL);
            coglink_add_track_to_queue(c_client, player,
                                       search_response->array[0]->encoded);
            break;
        }
        struct discord_embed embed = {
            .title = title,
            .description = description,
            .image =
                &(struct discord_embed_image){
                    .url = search_response->array[0]->info->artworkUrl,
                },
            .footer =
                &(struct discord_embed_footer){
                    .text = author,
                    .icon_url = LYRICAL_URL,
                },
            .timestamp = discord_timestamp(client),
        };

        struct discord_interaction_response params = {
            .type = DISCORD_INTERACTION_CHANNEL_MESSAGE_WITH_SOURCE,
            .data =
                &(struct discord_interaction_callback_data){
                    .embeds =
                        &(struct discord_embeds){
                            .size = 1,
                            .array = &embed,
                        },
                    .components =
                        &(struct discord_components){
                            .size = 1,
                            .array = &action_rows,
                        },
                },

        };
        discord_create_interaction_response(client, event->id, event->token,
                                            &params, NULL);
        coglink_add_track_to_queue(c_client, player,
                                   search_response->array[0]->encoded);
        break;
    }
    case COGLINK_LOAD_TYPE_EMPTY: {
        struct discord_embed embed = {
            .title = "No tracks found...",
            .description =
                "No tracks were found for the search query, please try again.",
            .footer =
                &(struct discord_embed_footer){
                    .icon_url = LYRICAL_URL,
                },
            .timestamp = discord_timestamp(client),
        };
        struct discord_interaction_response params = {
            .type = DISCORD_INTERACTION_CHANNEL_MESSAGE_WITH_SOURCE,
            .data =
                &(struct discord_interaction_callback_data){
                    .flags = DISCORD_MESSAGE_EPHEMERAL,
                    .embeds =
                        &(struct discord_embeds){
                            .size = 1,
                            .array = &embed,
                        },
                },
        };
        discord_create_interaction_response(client, event->id, event->token,
                                            &params, NULL);
        break;
    }
    case COGLINK_LOAD_TYPE_ERROR: {
        struct coglink_load_tracks_error *data = response.data;
        char description[4000 + 1];
        snprintf(description, sizeof(description), "Failed to load. %s",
                 data->message);

        struct discord_embed embed = {
            .title = "Failed to load...",
            .description = description,
            .footer =
                &(struct discord_embed_footer){
                    .icon_url = LYRICAL_URL,
                },
            .timestamp = discord_timestamp(client),
        };
        struct discord_interaction_response params = {
            .type = DISCORD_INTERACTION_CHANNEL_MESSAGE_WITH_SOURCE,
            .data =
                &(struct discord_interaction_callback_data){
                    .flags = DISCORD_MESSAGE_EPHEMERAL,
                    .embeds =
                        &(struct discord_embeds){
                            .size = 1,
                            .array = &embed,
                        },
                },
        };
        discord_create_interaction_response(client, event->id, event->token,
                                            &params, NULL);
        break;
    }
    }
    coglink_free_load_tracks(&response);
}
void skip_song(struct discord *client, const struct discord_interaction *event, struct coglink_client *c_client) {
    char title[64 + 1];
    char description[4000 + 1];
    char author[64 + 1];

    struct coglink_player *player =
        coglink_create_player(c_client, event->guild_id);
    if (!player) {
        struct discord_embed embed = {
            .timestamp = discord_timestamp(client),
            .title = "Failed to get the node.",
        };
        struct discord_interaction_response params = {
            .type = DISCORD_INTERACTION_CHANNEL_MESSAGE_WITH_SOURCE,
            .data =
                &(struct discord_interaction_callback_data){
                    .flags = DISCORD_MESSAGE_EPHEMERAL,
                    .embeds =
                        &(struct discord_embeds){
                            .size = 1,
                            .array = &embed,
                        },
                },
        };
        discord_create_interaction_response(client, event->id, event->token,
                                            &params, NULL);
        return;
    }
    struct coglink_player_queue *queue =
        coglink_get_player_queue(c_client, player);

    if (queue->size == 0) {
        struct discord_embed embed = {
            .title = "No tracks to skip...",
            .footer =
                &(struct discord_embed_footer){
                    .icon_url = LYRICAL_URL,
                },
            .timestamp = discord_timestamp(client),
        };
        struct discord_interaction_response params = {
            .type = DISCORD_INTERACTION_CHANNEL_MESSAGE_WITH_SOURCE,
            .data =
                &(struct discord_interaction_callback_data){
                    .flags = DISCORD_MESSAGE_EPHEMERAL,
                    .embeds =
                        &(struct discord_embeds){
                            .size = 1,
                            .array = &embed,
                        },
                },
        };
        discord_create_interaction_response(client, event->id, event->token,
                                            &params, NULL);
        return;
    }
    if (queue->size == 1) {
        coglink_remove_player(c_client, player);
        coglink_destroy_player(c_client, player);
        coglink_leave_voice_channel(c_client, client, event->guild_id);
        struct discord_embed embed = {
            .title = "Skipped...",
            .footer =
                &(struct discord_embed_footer){
                    .icon_url = LYRICAL_URL,
                },
            .timestamp = discord_timestamp(client),
        };
        struct discord_interaction_response params = {
            .type = DISCORD_INTERACTION_CHANNEL_MESSAGE_WITH_SOURCE,
            .data =
                &(struct discord_interaction_callback_data){
                    .flags = DISCORD_MESSAGE_EPHEMERAL,
                    .embeds =
                        &(struct discord_embeds){
                            .size = 1,
                            .array = &embed,
                        },
                },
        };
        discord_create_interaction_response(client, event->id, event->token,
                                            &params, NULL);
        return;
    }
    struct coglink_update_player response = {0};
    struct coglink_update_player_params next_track = {
        .paused = COGLINK_PAUSED_STATE_FALSE,
        .track = &(struct coglink_update_player_track_params){
            .encoded = queue->array[1]}};
    is_paused = false;
    coglink_update_player(c_client, player, &next_track, &response);
    coglink_remove_track_from_queue(c_client, player, 0);
    snprintf(title, sizeof(title), "Now playing...");
    snprintf(description, sizeof(description), "Now playing `%s.`",
             response.track->info->title);
    snprintf(author, sizeof(author), "By: %s", response.track->info->author);
    struct discord_embed embed = {
        .title = title,
        .description = description,
        .image =
            &(struct discord_embed_image){
                .url = response.track->info->artworkUrl,
            },
        .footer =
            &(struct discord_embed_footer){
                .text = author,
                .icon_url = LYRICAL_URL,
            },
        .timestamp = discord_timestamp(client),
    };
    struct discord_interaction_response params = {
        .type = DISCORD_INTERACTION_CHANNEL_MESSAGE_WITH_SOURCE,
        .data =
            &(struct discord_interaction_callback_data){
                .components = &(struct discord_components){
                    .size = 1,
                    .array = &action_rows,
                },
                .embeds = &(struct discord_embeds){
                    .size = 1,
                    .array = &embed,
                },
            },
    };
    discord_create_interaction_response(client, event->id, event->token, &params,
                                        NULL);
    return;
}
void stop(struct discord *client, const struct discord_interaction *event, struct coglink_client *c_client) {
    struct coglink_player *player =
        coglink_create_player(c_client, event->guild_id);
    if (!player) {
        struct discord_embed embed = {
            .timestamp = discord_timestamp(client),
            .title = "Failed to get the node.",
        };
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
        discord_create_interaction_response(client, event->id, event->token,
                                            &params, NULL);
        return;
    }
    if (coglink_remove_player(c_client, player) == COGLINK_FAILED) {
        struct discord_embed embed = {
            .title = "Failed...",
            .description = "Failed to remove the player.",
            .footer =
                &(struct discord_embed_footer){
                    .icon_url = LYRICAL_URL,
                },
            .timestamp = discord_timestamp(client),
        };
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
        discord_create_interaction_response(client, event->id, event->token,
                                            &params, NULL);
        return;
    }
    coglink_destroy_player(c_client, player);

    if (coglink_leave_voice_channel(c_client, client, event->guild_id)) {
        struct discord_embed embed = {
            .title = "Failed...",
            .description = "Failed to disconnect from the voice channel.",
            .footer =
                &(struct discord_embed_footer){
                    .icon_url = LYRICAL_URL,
                },
            .timestamp = discord_timestamp(client),
        };
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
        discord_create_interaction_response(client, event->id, event->token,
                                            &params, NULL);
        return;
    }
    struct discord_embed embed = {
        .title = "Left the voice channel.",
        .footer =
            &(struct discord_embed_footer){
                .icon_url = LYRICAL_URL,
            },
        .timestamp = discord_timestamp(client),
    };
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
    is_paused = true;
    discord_create_interaction_response(client, event->id, event->token, &params,
                                        NULL);
    return;
}
void pause_play(struct discord *client, const struct discord_interaction *event, struct coglink_client *c_client) {
    struct coglink_player *player =
        coglink_create_player(c_client, event->guild_id);
    if (!player) {
        struct discord_embed embed = {
            .timestamp = discord_timestamp(client),
            .title = "Failed to get the node...",
        };
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
        discord_create_interaction_response(client, event->id, event->token,
                                            &params, NULL);
        return;
    }
    struct coglink_update_player_params pause = {
        .paused = COGLINK_PAUSED_STATE_TRUE,
    };

    struct coglink_update_player_params resume = {
        .paused = COGLINK_PAUSED_STATE_FALSE,
    };

    if (is_paused) {
        coglink_update_player(c_client, player, &resume, NULL);
        is_paused = false;
        struct discord_embed embed = {
            .timestamp = discord_timestamp(client),
            .title = "Resumed...",
        };
        struct discord_interaction_response params = {
            .type = DISCORD_INTERACTION_CHANNEL_MESSAGE_WITH_SOURCE,
            .data =
                &(struct discord_interaction_callback_data){
                    .flags = DISCORD_MESSAGE_EPHEMERAL,
                    .embeds =
                        &(struct discord_embeds){
                            .size = 1,
                            .array = &embed,
                        },
                },
        };
        discord_create_interaction_response(client, event->id, event->token,
                                            &params, NULL);
        return;
    } else {
        coglink_update_player(c_client, player, &pause, NULL);
        is_paused = true;
        struct discord_embed embed = {
            .timestamp = discord_timestamp(client),
            .title = "Paused...",
        };
        struct discord_interaction_response params = {
            .type = DISCORD_INTERACTION_CHANNEL_MESSAGE_WITH_SOURCE,
            .data =
                &(struct discord_interaction_callback_data){
                    .flags = DISCORD_MESSAGE_EPHEMERAL,
                    .embeds =
                        &(struct discord_embeds){
                            .size = 1,
                            .array = &embed,
                        },
                },
        };
        discord_create_interaction_response(client, event->id, event->token,
                                            &params, NULL);
        return;
    }
    return;
}
void rejoin(struct discord *client, const struct discord_interaction *event, struct coglink_client *c_client) {
    struct coglink_user *user =
        coglink_get_user(c_client, event->member->user->id);
    if (user == NULL) {
        struct discord_embed embed = {
            .timestamp = discord_timestamp(client),
            .title = "You are not in a channel...",
        };
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
        discord_create_interaction_response(client, event->id, event->token,
                                            &params, NULL);
        return;
    }
    coglink_join_voice_channel(c_client, client, event->guild_id,
                               user->channel_id);
    struct discord_embed embed = {
        .timestamp = discord_timestamp(client),
        .title = "Rejoined...",
    };
    struct discord_interaction_response params = {
        .type = DISCORD_INTERACTION_CHANNEL_MESSAGE_WITH_SOURCE,
        .data =
            &(struct discord_interaction_callback_data){
                .flags = DISCORD_MESSAGE_EPHEMERAL,
                .embeds =
                    &(struct discord_embeds){
                        .size = 1,
                        .array = &embed,
                    },
            },
    };
    discord_create_interaction_response(client, event->id, event->token, &params,
                                        NULL);
    return;
}
void get_queue(struct discord *client, const struct discord_interaction *event, struct coglink_client *c_client) {
    struct coglink_player *player =
        coglink_create_player(c_client, event->guild_id);
    if (!player) {
        struct discord_embed embed = {
            .timestamp = discord_timestamp(client),
            .title = "Failed to get the node...",
        };
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
        discord_create_interaction_response(client, event->id, event->token,
                                            &params, NULL);
        return;
    }

    struct coglink_player_queue *queue =
        coglink_get_player_queue(c_client, player);

    if (queue->size == 0) {
        struct discord_embed embed = {
            .timestamp = discord_timestamp(client),
            .title = "The queue is empty...",
        };
        struct discord_interaction_response params = {
            .type = DISCORD_INTERACTION_CHANNEL_MESSAGE_WITH_SOURCE,
            .data =
                &(struct discord_interaction_callback_data){
                    .flags = DISCORD_MESSAGE_EPHEMERAL,
                    .embeds =
                        &(struct discord_embeds){
                            .size = 1,
                            .array = &embed,
                        },
                },
        };
        discord_create_interaction_response(client, event->id, event->token,
                                            &params, NULL);
        return;
    }

    struct coglink_decode_tracks_params decode_params = {
        .array = queue->array,
        .size = queue->size,
    };
    char *description = calloc(queue->size, 128 + 1);
    size_t description_size = queue->size * (128 + 1);

    struct coglink_node *node = coglink_get_player_node(c_client, player);
    struct coglink_tracks tracks = {0};
    coglink_decode_tracks(c_client, node, &decode_params, &tracks);

    for (size_t i = 0; i < queue->size; i++) {
        snprintf(description + strlen(description),
                 description_size - strlen(description), "%zu. `%.*s`\n", i + 1,
                 100, tracks.array[i]->info->title);
    }
    struct discord_embed embed = {
        .timestamp = discord_timestamp(client),
        .title = "Current queue",
        .description = description,
    };
    struct discord_interaction_response params = {
        .type = DISCORD_INTERACTION_CHANNEL_MESSAGE_WITH_SOURCE,
        .data =
            &(struct discord_interaction_callback_data){
                .flags = DISCORD_MESSAGE_EPHEMERAL,
                .embeds =
                    &(struct discord_embeds){
                        .size = 1,
                        .array = &embed,
                    },
            },
    };
    discord_create_interaction_response(client, event->id, event->token, &params,
                                        NULL);
    free(description);
    return;
}
void pop_queue(struct discord *client, const struct discord_interaction *event, struct coglink_client *c_client) {
    struct coglink_player *player =
        coglink_create_player(c_client, event->guild_id);
    if (!player) {
        struct discord_embed embed = {
            .timestamp = discord_timestamp(client),
            .title = "Failed to get the node...",
        };
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
        discord_create_interaction_response(client, event->id, event->token,
                                            &params, NULL);
        return;
    }
    struct coglink_player_queue *queue =
        coglink_get_player_queue(c_client, player);
    size_t position = (strtoul(event->data->options->array->value, NULL, 10) - 1);

    printf("\n%zu\n", position);
    if (position >= queue->size || position < 0) {
        struct discord_embed embed = {
            .timestamp = discord_timestamp(client),
            .title = "Invalid position...",
        };
        struct discord_interaction_response params = {
            .type = DISCORD_INTERACTION_CHANNEL_MESSAGE_WITH_SOURCE,
            .data =
                &(struct discord_interaction_callback_data){
                    .flags = DISCORD_MESSAGE_EPHEMERAL,
                    .embeds =
                        &(struct discord_embeds){
                            .size = 1,
                            .array = &embed,
                        },
                },
        };
        discord_create_interaction_response(client, event->id, event->token,
                                            &params, NULL);
        return;
    }
    coglink_remove_track_from_queue(c_client, player, position);
    struct discord_embed embed = {
        .timestamp = discord_timestamp(client),
        .title = "Removed from queue...",
    };
    struct discord_interaction_response params = {
        .type = DISCORD_INTERACTION_CHANNEL_MESSAGE_WITH_SOURCE,
        .data =
            &(struct discord_interaction_callback_data){
                .flags = DISCORD_MESSAGE_EPHEMERAL,
                .embeds =
                    &(struct discord_embeds){
                        .size = 1,
                        .array = &embed,
                    },
            },
    };
    discord_create_interaction_response(client, event->id, event->token, &params,
                                        NULL);
    return;
}
void on_track_end(struct coglink_client *c_client, struct coglink_node *node, struct coglink_track_end *trackEnd) {
    struct coglink_player *player =
        coglink_get_player(c_client, embed_event->guild_id);
    struct coglink_player_queue *queue =
        coglink_get_player_queue(c_client, player);
    struct coglink_tracks response = {0};
    coglink_decode_tracks(c_client, node,
                          &(struct coglink_decode_tracks_params){
                              .array = queue->array, .size = queue->size},
                          &response);
    log_info("Track ended, Next track`: %s", response.array[0]->info->title);
    char title[64 + 1];
    char description[4000 + 1];
    char author[64 + 1];
    if (queue->size == 0) {
        snprintf(title, sizeof(title), "Now Playing...");
        struct discord_embed embed = {
            .title = title,
            .description = "No tracks in the queue...",
            .image =
                &(struct discord_embed_image){
                    .url = LYRICAL_URL,
                },
            .footer =
                &(struct discord_embed_footer){
                    .text = author,
                    .icon_url = LYRICAL_URL,
                },
            .timestamp = discord_timestamp(CLIENT),
        };
        struct discord_edit_original_interaction_response edited_message = {
            .embeds =
                &(struct discord_embeds){
                    .size = 1,
                    .array = &embed,
                },
            .components =
                &(struct discord_components){
                    .size = 1,
                    .array = &action_rows,
                },
        };
        discord_edit_original_interaction_response(
            CLIENT, embed_event->application_id, embed_event->token, &edited_message,
            NULL);
    } else {
        snprintf(title, sizeof(title), "Now Playing...");
        snprintf(description, sizeof(description), "`%s.`",
                 response.array[0]->info->title);
        struct discord_embed embed = {
            .title = title,
            .description = description,
            .image =
                &(struct discord_embed_image){
                    .url = response.array[0]->info->artworkUrl,
                },
            .footer =
                &(struct discord_embed_footer){
                    .text = author,
                    .icon_url = LYRICAL_URL,
                },
            .timestamp = discord_timestamp(CLIENT),
        };
        struct discord_edit_original_interaction_response edited_message = {
            .embeds =
                &(struct discord_embeds){
                    .size = 1,
                    .array = &embed,
                },
            .components =
                &(struct discord_components){
                    .size = 1,
                    .array = &action_rows,
                },
        };
        discord_edit_original_interaction_response(
            CLIENT, embed_event->application_id, embed_event->token, &edited_message,
            NULL);
    }
}

/*

TO DO:
- FREE MEMEORY ON STOP COMMAND
- Refactor all the commands to use update interactions
- USE DISCORD_INTERACTION_UPDATE_MESSAGE TO UPDATE MESSAGES

*/