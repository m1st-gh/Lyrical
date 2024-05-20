#include "lyrical.h"

#include <coglink/types.h>
#include <coglink/utils.h>
#include <coglink/websocket.h>
#include <concord/channel.h>
#include <concord/discord-events.h>
#include <concord/discord.h>
#include <concord/discord_codecs.h>
#include <concord/interaction.h>
#include <concord/log.h>
#include <concord/types.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

void handle_sigsegv(int sig) {
    // Print an error message
    log_error("Segmentation fault caught! Exiting...");

    // Perform cleanup operations here
    ccord_shutting_down();
    // Exit the program
    exit(sig);
}

struct coglink_client *C_CLIENT;
struct discord *CLIENT;

u64snowflake GUILD_ID;
u64snowflake APP_ID;
u64snowflake BOT_ID;
char NODE_PASS[33];
char NODE_IP[16];
char NODE_PORT[6];

void lyrical_ready(struct coglink_client *client, struct coglink_node *node,
                   struct coglink_ready *ready) {
    (void)client;
    (void)node;

    log_info("[COGLINK] Node connected [%s]", ready->session_id);
}

void lyrical_stats(struct coglink_client *client, struct coglink_node *node,
                   struct coglink_stats *stats) {
    (void)client;
    (void)node;
    printf("\n\n\nI AM GETING RUN\n\n\n");
    printf("InfoMemory:\n  > Free: %d\n  > Used: %d\n  > Reservable: %d\n",
           stats->memory->free, stats->memory->used, stats->memory->reservable);
    printf("InfoCPU:\n  > Cores: %d\n  > SystemLoad: %d\n  > LavalinkLoad: %d\n",
           stats->cpu->cores, stats->cpu->systemLoad, stats->cpu->lavalinkLoad);
    if (stats->frameStats) {
        printf("InfoFrameStats:\n  > Sent: %d\n  > Nulled: %d\n  > Deficit: %d\n",
               stats->frameStats->sent, stats->frameStats->nulled,
               stats->frameStats->deficit);
    }
    printf("PlayingPlayers: %d\nPlayers: %d\nUptime: %d\n", stats->playingPlayers,
           stats->players, stats->uptime);
}

void on_ready(struct discord *client, const struct discord_ready *event) {
    log_info("Lyrical succesfully connected to Discord as %s#%s!",
             event->user->username, event->user->discriminator);
    APP_ID = event->application->id;

    /*create as union of gloabal commands*/
    struct discord_application_command_option song_options[] = {{
        .type = DISCORD_APPLICATION_OPTION_STRING,
        .name = "track",
        .description =
            "The track to play. Can be a URL, search query, or playlist URL.",
        .required = true,
    }};
    struct discord_application_command_option position_options[] = {{
        .type = DISCORD_APPLICATION_OPTION_INTEGER,
        .name = "position",
        .description = "The position in the queue to skip to.",
        .required = true,
    }};
    struct discord_create_guild_application_command play = {
        .name = "play",
        .description = "Plays a track, playlist, or searchs for a track.",
        .options =
            &(struct discord_application_command_options){
                .array = song_options,
                .size = sizeof(song_options) / sizeof(*song_options)},
    };
    struct discord_create_guild_application_command get_queue = {
        .name = "queue",
        .description = "Retrieves the current queue.",
    };
    struct discord_create_guild_application_command pop_queue = {
        .name = "pop",
        .description =
            "Pops the given track number, use /queue to find track numbers.",
        .options =
            &(struct discord_application_command_options){
                .array = position_options,
                .size = sizeof(position_options) / sizeof(*position_options)},
    };

    discord_create_guild_application_command(client, APP_ID, GUILD_ID, &play, NULL);
    discord_create_guild_application_command(client, APP_ID, GUILD_ID, &get_queue, NULL);
    discord_create_guild_application_command(client, APP_ID, GUILD_ID, &pop_queue, NULL);
}

void get_config_feilds(struct discord *client, char *field, char *subfield,
                       char *dest) {
    struct ccord_szbuf_readonly field_buf =
        discord_config_get_field(client, (char *[2]){field, subfield}, 2);
    strncpy(dest, field_buf.start, field_buf.size);
}

struct discord *init_bot() {
    struct discord *client = discord_config_init("config.json");
    char Temp[64];
    get_config_feilds(client, "server", "guild_id", Temp);
    GUILD_ID = strtoul(Temp, NULL, 10);
    get_config_feilds(client, "server", "node_ip", NODE_IP);
    get_config_feilds(client, "server", "node_pass", NODE_PASS);
    get_config_feilds(client, "server", "node_port", NODE_PORT);
    get_config_feilds(client, "server", "bot_id", Temp);
    BOT_ID = strtoul(Temp, NULL, 10);
    return client;
}

void on_interaction(struct discord *client,
                    const struct discord_interaction *event) {
    switch (event->type) {
    case DISCORD_INTERACTION_PING:
    case DISCORD_INTERACTION_APPLICATION_COMMAND:
        if (strcmp(event->data->name, "play") == 0) {
            play_song(client, event, C_CLIENT);
            break;
        } else if (strcmp(event->data->name, "rejoin") == 0) {
            rejoin(client, event, C_CLIENT);
            break;
        } else if (strcmp(event->data->name, "pop") == 0) {
            pop_queue(client, event, C_CLIENT);
            break;
        }
        break;
    case DISCORD_INTERACTION_MESSAGE_COMPONENT:
        if (strcmp(event->data->custom_id, "skip_button") == 0) {
            skip_song(client, event, C_CLIENT);
            break;
        } else if (strcmp(event->data->custom_id, "pause_play_button") == 0){
            pause_play(client, event, C_CLIENT);
            break;
        } else if (strcmp(event->data->custom_id, "stop_button") == 0) {
            stop(client, event, C_CLIENT);
            break;
        } else if (strcmp(event->data->custom_id, "queue_button") == 0) {
            get_queue(client, event, C_CLIENT);
            break;
        }
        break;
    case DISCORD_INTERACTION_APPLICATION_COMMAND_AUTOCOMPLETE:
        break;
    case DISCORD_INTERACTION_MODAL_SUBMIT:
        break;
    default:
        break;
    }
}

int main() {
    signal(SIGSEGV, handle_sigsegv);
    struct discord *client = init_bot();
    CLIENT = client;
    ccord_global_init();

    C_CLIENT = malloc(sizeof(struct coglink_client));
    C_CLIENT->bot_id = BOT_ID;
    C_CLIENT->num_shards = "1";
    struct coglink_nodes nodes = {
        .array = (struct coglink_node[]){{.name = "Node 1",
                                          .hostname = NODE_IP,
                                          .port = atoi(NODE_PORT),
                                          .password = NODE_PASS,
                                          .ssl = false}},
        .size = 1};

    C_CLIENT->events = &(struct coglink_events){
        .on_ready = &lyrical_ready,
        .on_stats = &lyrical_stats,
        .on_track_end = &on_track_end,
    };
    coglink_connect_nodes(C_CLIENT, client, &nodes);
    discord_add_intents(client, DISCORD_GATEWAY_GUILD_VOICE_STATES);
    discord_add_intents(client, DISCORD_GATEWAY_MESSAGE_CONTENT);
    discord_add_intents(client, DISCORD_GATEWAY_GUILDS);
    discord_set_on_ready(client, &on_ready);
    discord_set_on_interaction_create(client, &on_interaction);
    discord_run(client);
    discord_cleanup(client);
    coglink_cleanup(C_CLIENT);
    ccord_global_cleanup();
}