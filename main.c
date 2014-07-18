#include <gtk/gtk.h>
#include <stdio.h>
#include <string.h>

#define array_size(x) (sizeof(x)/sizeof(x[0]))

const char * const video_driver[] = {"X11Grab", "Video4Linux2"};
const char * const audio_driver[] = {"ALSA", "Pulse", "OSS"};

enum {COMBO_VIDEO_DRIVER, TEXT_VIDEO_DEVICE, COMBO_AUDIO_DRIVER, 
  TEXT_AUDIO_DEVICE, TEXT_INPUT_RES, INT_FPS, INT_AUDIO_RATE,
  INT_AUDIO_CHANNELS, TEXT_OUTPUT_RES, INT_AUDIO_BITRATE, INT_VIDEO_BITRATE, 
  COMBO_QUALITY, INT_THREADS, TEXT_STREAM_KEY, BTN_CONTROL};
GtkWidget *widgets[15];

pid_t child_pid = -1;


static void destroy(GtkWidget *widget, gpointer data) {
  gtk_main_quit();
}

static void start_stream(GtkWidget *widget, gpointer data) {
  // check if already running
  if (child_pid == -1) {
    child_pid = fork();
    // the child will exec ffmpeg
    if (child_pid == 0) {
      // convert everything to strings
      int ifps = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(widgets[INT_FPS]));
      int iaudio_bitrate = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(widgets[INT_AUDIO_BITRATE]));
      int ivideo_bitrate = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(widgets[INT_VIDEO_BITRATE]));
      char fps[5];
      sprintf(fps, "%i", ifps);
      char gop[6];
      sprintf(gop, "%i", ifps * 2);
      char audio_bitrate[15];
      sprintf(audio_bitrate, "%ik", iaudio_bitrate);
      char video_bitrate[15];
      sprintf(video_bitrate, "%ik", ivideo_bitrate);
      char total_bitrate[15];
      sprintf(total_bitrate, "%ik", iaudio_bitrate + ivideo_bitrate);
      char ac[4];
      sprintf(ac, "%i", gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(widgets[INT_AUDIO_CHANNELS])));
      char ar[4];
      sprintf(ar, "%i", gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(widgets[INT_AUDIO_RATE])));
      char threads[4];
      sprintf(threads, "%i", gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(widgets[INT_THREADS])));
      char url[200];
      strcpy(url, "rtmp://live.twitch.tv/app/"); 
      strcat(url, gtk_entry_get_text(GTK_ENTRY(widgets[TEXT_STREAM_KEY])));


      execl("/usr/bin/ffmpeg", "/usr/bin/ffmpeg", 
          "-f", gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(widgets[COMBO_VIDEO_DRIVER])),
          "-s", gtk_entry_get_text(GTK_ENTRY(widgets[TEXT_INPUT_RES])),
          "-r", fps,
          "-i", gtk_entry_get_text(GTK_ENTRY(widgets[TEXT_VIDEO_DEVICE])),
          "-f", gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(widgets[COMBO_AUDIO_DRIVER])),
          "-i", gtk_entry_get_text(GTK_ENTRY(widgets[TEXT_AUDIO_DEVICE])),
          "-ac", ac,
          "-ar", ar,
          "-vcodec", "libx264",
          "-g", gop,
          "-keyint_min", fps,     //GOP min
          "-b:a", audio_bitrate,
          "-b:v", video_bitrate,
          "-minrate", total_bitrate,
          "-maxrate", total_bitrate,
          "-pix_fmt", "yuv420p",
          "-s", gtk_entry_get_text(GTK_ENTRY(widgets[TEXT_OUTPUT_RES])),
          "-preset", gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(widgets[COMBO_QUALITY])),
          "-tune", "film",
          "-acodec", "libmp3lame",
          "-threads", threads,
          "-strict", "normal",
          "-bufsize", total_bitrate,
          "-f", "flv",
          url, NULL);

       perror("Exec failed");
      _exit(0);
    } else {
      gtk_button_set_label(GTK_BUTTON(widgets[BTN_CONTROL]), "Stop");
    }
  } else {
    kill(child_pid, SIGTERM);
    child_pid = -1;
    gtk_button_set_label(GTK_BUTTON(widgets[BTN_CONTROL]), "Start");
  }
}

static void vid_driver_change(GtkWidget *widget, gpointer data) {
  switch (gtk_combo_box_get_active(GTK_COMBO_BOX(widget))) {
    case 0:
      gtk_entry_set_text(GTK_ENTRY(widgets[TEXT_VIDEO_DEVICE]), ":0");
      break;
    case 1:
      gtk_entry_set_text(GTK_ENTRY(widgets[TEXT_VIDEO_DEVICE]), "/dev/video0");
      break;
    default:
      break;
  }
}

static void audio_driver_change(GtkWidget *widget, gpointer data) {
  switch (gtk_combo_box_get_active(GTK_COMBO_BOX(widget))) {
    case 0:
      gtk_entry_set_text(GTK_ENTRY(widgets[TEXT_AUDIO_DEVICE]), "hw:0");
      break;
    case 1:
      gtk_entry_set_text(GTK_ENTRY(widgets[TEXT_AUDIO_DEVICE]), "default");
      break;
    case 2:
      gtk_entry_set_text(GTK_ENTRY(widgets[TEXT_AUDIO_DEVICE]), "/dev/dsp");
      break;
    default:
      break;
  }
}

static void add_widget(GtkWidget *widget, gchar *label, GtkGrid *layout) {
  GtkWidget *l = gtk_label_new(label);
  gtk_container_add(GTK_CONTAINER(layout), l);
  gtk_grid_attach_next_to(layout, widget, l, GTK_POS_RIGHT, 1, 1);
}

int main (int argc, char *argv[]) {
  gtk_init(&argc, &argv);

  GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  gtk_window_set_title(GTK_WINDOW(window), "Stwitch");
  g_signal_connect(window, "destroy", G_CALLBACK(destroy), NULL);
  gtk_container_set_border_width(GTK_CONTAINER(window), 20);

  GtkWidget *layout = gtk_grid_new();
  gtk_grid_set_row_spacing(GTK_GRID(layout), 3);
  gtk_grid_set_column_spacing(GTK_GRID(layout), 10);
  gtk_orientable_set_orientation(GTK_ORIENTABLE(layout), GTK_ORIENTATION_VERTICAL);
  gtk_container_add(GTK_CONTAINER(window), layout);

  widgets[COMBO_VIDEO_DRIVER] = gtk_combo_box_text_new();
  for (int i = 0; i < array_size(video_driver); ++i) {
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(widgets[COMBO_VIDEO_DRIVER]), 
        video_driver[i], video_driver[i]);
  }
  g_signal_connect(GTK_COMBO_BOX_TEXT(widgets[COMBO_VIDEO_DRIVER]), "changed", 
      G_CALLBACK(vid_driver_change), NULL);
  add_widget(widgets[COMBO_VIDEO_DRIVER], "Video driver", GTK_GRID(layout));

  widgets[TEXT_VIDEO_DEVICE] = gtk_entry_new();
  add_widget(widgets[TEXT_VIDEO_DEVICE], "Video device", GTK_GRID(layout));

  widgets[TEXT_INPUT_RES] = gtk_entry_new();
  gtk_entry_set_text(GTK_ENTRY(widgets[TEXT_INPUT_RES]), "1280x720");
  add_widget(widgets[TEXT_INPUT_RES], "Input resolution", GTK_GRID(layout));

  GtkAdjustment *fps_adj = gtk_adjustment_new(30, 5, 999, 5, 1, 1);
  widgets[INT_FPS] = gtk_spin_button_new(fps_adj, 1, 0);
  add_widget(widgets[INT_FPS], "Frames per second", GTK_GRID(layout));

  widgets[COMBO_AUDIO_DRIVER] = gtk_combo_box_text_new();
  for (int i = 0; i < array_size(audio_driver); ++i) {
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(widgets[COMBO_AUDIO_DRIVER]), 
        audio_driver[i], audio_driver[i]);
  }
  g_signal_connect(GTK_COMBO_BOX_TEXT(widgets[COMBO_AUDIO_DRIVER]), "changed", 
      G_CALLBACK(audio_driver_change), NULL);
  add_widget(widgets[COMBO_AUDIO_DRIVER], "Audio driver", GTK_GRID(layout));

  widgets[TEXT_AUDIO_DEVICE] = gtk_entry_new();
  add_widget(widgets[TEXT_AUDIO_DEVICE], "Audio device", GTK_GRID(layout));

  GtkAdjustment *rate = gtk_adjustment_new(44100, 800, 999999, 100, 1, 1);
  widgets[INT_AUDIO_RATE] = gtk_spin_button_new(rate, 1, 0);
  add_widget(widgets[INT_AUDIO_RATE], "Audio rate (Hz)", GTK_GRID(layout));

  GtkAdjustment *channels = gtk_adjustment_new(1, 1, 65, 1, 1, 1);
  widgets[INT_AUDIO_CHANNELS] = gtk_spin_button_new(channels, 1, 0);
  add_widget(widgets[INT_AUDIO_CHANNELS], "Audio channels", GTK_GRID(layout));

  widgets[TEXT_OUTPUT_RES] = gtk_entry_new();
  gtk_entry_set_text(GTK_ENTRY(widgets[TEXT_OUTPUT_RES]), "1280x720");
  add_widget(widgets[TEXT_OUTPUT_RES], "Output resolution", GTK_GRID(layout));

  GtkAdjustment *audio_bitrate = gtk_adjustment_new(128, 32, 1280, 32, 1, 1);
  widgets[INT_AUDIO_BITRATE] = gtk_spin_button_new(audio_bitrate, 1, 0);
  add_widget(widgets[INT_AUDIO_BITRATE], "Audio bitrate (k)", GTK_GRID(layout));

  GtkAdjustment *bitrate = gtk_adjustment_new(1000, 100, 100000, 100, 1, 1);
  widgets[INT_VIDEO_BITRATE] = gtk_spin_button_new(bitrate, 1, 0);
  add_widget(widgets[INT_VIDEO_BITRATE], "Video bitrate (k)", GTK_GRID(layout));

  widgets[COMBO_QUALITY] = gtk_combo_box_text_new();
  gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(widgets[COMBO_QUALITY]),
      "ultrafast", "ultrafast");
  gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(widgets[COMBO_QUALITY]),
      "superfast", "superfast");
  gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(widgets[COMBO_QUALITY]),
      "faster", "faster");
  gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(widgets[COMBO_QUALITY]),
      "fast", "fast");
  gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(widgets[COMBO_QUALITY]),
      "medium", "medium");
  gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(widgets[COMBO_QUALITY]),
      "slow", "slow");
  gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(widgets[COMBO_QUALITY]),
      "slower", "slower");
  gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(widgets[COMBO_QUALITY]),
      "veryslow", "veryslow");
  gtk_combo_box_set_active(GTK_COMBO_BOX(widgets[COMBO_QUALITY]), 0);
  add_widget(widgets[COMBO_QUALITY], "Quality", GTK_GRID(layout));

  GtkAdjustment *threads = gtk_adjustment_new(2, 1, 17, 1, 1, 1);
  widgets[INT_THREADS] = gtk_spin_button_new(threads, 1, 0);
  add_widget(widgets[INT_THREADS], "Threads", GTK_GRID(layout));

  widgets[TEXT_STREAM_KEY] = gtk_entry_new();
  add_widget(widgets[TEXT_STREAM_KEY], "Stream key", GTK_GRID(layout));

  widgets[BTN_CONTROL] = gtk_button_new_with_label("Start");
  g_signal_connect(GTK_BUTTON(widgets[BTN_CONTROL]), "clicked", G_CALLBACK(start_stream), NULL);
  add_widget(widgets[BTN_CONTROL], "Start stream", GTK_GRID(layout));

  gtk_widget_show_all(window);

  gtk_main();

  return 0;
}

