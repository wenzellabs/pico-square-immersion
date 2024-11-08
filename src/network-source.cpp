#include "pico/critical_section.h"
#include <network-source.hpp>
#include "pico/unique_id.h"
#include <TLV_registry.hpp>
#include <wifi-stuff.hpp>
#include <display.h>
#include <stdio.h>
#include <ntp.hpp>
#include <tlv.h>
#include "led.h"


const char *scale_name[] = {
  "major",
  "minor",
  "harmonic_minor",
  "melodic_minor",
  "dorian",
  "phrygian",
  "lydian",
  "mixolydian",
  "locrian",
  "major_pentatonic",
  "minor_pentatonic",
  "blues_minor",
  "blues_major",
  "whole_tone",
  "chromatic",
};

const uint8_t scale_off[][13] = {
  {7, 0, 2, 4, 5, 7, 9, 11}, // first byte is note count
  {7, 0, 2, 3, 5, 7, 8, 10},
  {7, 0, 2, 3, 5, 7, 8, 11},
  {7, 0, 2, 3, 5, 7, 9, 11},
  {7, 0, 2, 3, 5, 7, 9, 10},
  {7, 0, 1, 3, 5, 7, 8, 10},
  {7, 0, 2, 4, 6, 7, 9, 11},
  {7, 0, 2, 4, 5, 7, 9, 10},
  {7, 0, 1, 3, 5, 6, 8, 10},
  {5, 0, 2, 4, 7, 9},
  {5, 0, 3, 5, 7, 10},
  {6, 0, 3, 5, 6, 7, 10},
  {6, 0, 3, 4, 7, 9, 10},
  {6, 0, 2, 4, 6, 8, 10},
  {12, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11},
};

const char *note_name[12] =
{
  "C",
  "C#",
  "D",
  "D#",
  "E",
  "F",
  "F#",
  "G",
  "G#",
  "A",
  "A#",
  "B",
};

Network_source::Network_source(MIDI_state_machine *const midi_state_machine) :
    _midi_state_machine(midi_state_machine)
{
  init_tlv(_tlv_reg);
  int ret = init_wifi_stuff();
  if (ret != 0)
  {
    printf("no WiFi\n");
    memset(display_buffer, 0, SSD1306_BUF_LEN);
    write_string(2, 8, "no WiFi :(");
    write_string(38, is_large_display()?56:24, "[" NICK "]");
    render(&full_frame_area);
    has_wifi = false;
  }else{
    has_wifi = true;
  }

  memset(&note_buffer, 0, sizeof(note_buffer));
  uint64_t uniq_id;
  pico_get_unique_board_id((pico_unique_board_id_t *)(&uniq_id));
  printf("uniq id: 0x%llx\n", uniq_id);
}

Network_source::~Network_source()
{
  close_wifi_stuff();
}

void Network_source::set_ntp(NTP_client *const ntp)
{
  _ntp = ntp;
}

void Network_source::rx_task()
{
    while (udp_buffer.count > 0) {
        uint8_t *packet = udp_buffer.data[udp_buffer.tail];

        // Call run_callbacks on the _tlv_reg instance
        _tlv_reg.run_callbacks((tlv_packet_t *)packet);

        udp_buffer.tail = (udp_buffer.tail + 1) % UDP_BUFFER_SIZE;
        uint32_t irq_state = save_and_disable_interrupts();
        udp_buffer.count--;
        restore_interrupts(irq_state);
    }
    play_note();
}

void Network_source::play_note()
{
  uint64_t now = _ntp->powerup_time + get_absolute_time();
  if (note_buffer.count > 0)
  {
    while ((note_buffer.note[note_buffer.head].us_since_1900 < now ) &&
          ( note_buffer.count > 0 ))
    {
      //printf("%llu < %llu\n", note_buffer.note[note_buffer.head].us_since_1900, now);

      tlv_type_note_t *p = &(note_buffer.note[note_buffer.head]);

      printf("playing note %s%d o%s %lld ms late\n",
              note_name[p->note%12], p->note/12-1,
              p->onoff ? "n" : "ff",
              (now  - note_buffer.note[note_buffer.head].us_since_1900) / 1000LL);

      uint8_t packet[4];
      if (p->onoff == 1)
      {
        packet[0] = 0x09;  // note on

        scroll_down(8);
        char str_note[6];
        snprintf(str_note, 6, "%s%d", note_name[p->note%12], p->note/12-1);
        write_string(2, 0, str_note);

        draw_rect(32, 0, SSD1306_WIDTH-1, SSD1306_HEIGHT-1, false);

        int yy = 0;
        write_string(32, yy, _artist);
        if (strlen(_artist) > 16)
        {
          yy += 8;
          write_string(32, yy, _artist+16);
        }
        if (strlen(_artist) > 32)
        {
          yy += 8;
          write_string(32, yy, _artist+32);
        }
        yy += 8;
        write_string(32, yy, _title);
        if (strlen(_title) > 16)
        {
          yy += 8;
          write_string(32, yy, _title+16);
        }
        if (strlen(_title) > 32)
        {
          yy += 8;
          write_string(32, yy, _title+32);
        }
        if (strlen(_title) > 48)
        {
          yy += 8;
          write_string(32, yy, _title+48);
        }

        write_string(38, is_large_display()?56:24, "[" NICK "]");
        render_full();

        scroll_down_leds();
        update_leds();

      }else{
        packet[0] = 0x08;  // note off
      }
      packet[1] = p->channel; // channel
      packet[2] = p->note;
      packet[3] = p->velocity;

      _midi_state_machine->consume_event_packet(packet);

      note_buffer.head = (note_buffer.head + 1) % NOTE_BUFFER_SIZE;
      note_buffer.count--;
    }
  }
}

void Network_source::tlv_time(tlv_packet_t *tp)
{
  tlv_type_time_t *p = (tlv_type_time_t *)tp->payload;
  int64_t delta = _ntp->powerup_time + get_absolute_time() - p->us_since_1900;
  printf("his master's clock is off by %llu\n", delta);
}

void Network_source::enqueue_note(tlv_packet_t *tp, uint8_t onoff)
{
    // packet to note
    tlv_type_note_on_t *p = (tlv_type_note_on_t *)tp->payload;

    tlv_type_note_t new_note;
    new_note.onoff = onoff;
    new_note.us_since_1900 = p->us_since_1900;
    new_note.note = p->note;
    new_note.channel = p->channel;
    new_note.velocity = p->velocity;

    // buffer full? discard oldest event (move head forward)
    if (note_buffer.count == NOTE_BUFFER_SIZE) {
        note_buffer.head = (note_buffer.head + 1) % NOTE_BUFFER_SIZE;
        note_buffer.count--;
        printf("WARNING: dropping note, circular buffer is full\n");
    }

    // buffer empty? -> insert note at tail, return
    if (note_buffer.count == 0) {
        note_buffer.note[note_buffer.tail] = new_note; // copy
        note_buffer.tail = (note_buffer.tail + 1) % NOTE_BUFFER_SIZE;
        note_buffer.count++;
        return;
    }

    // binary search for the correct insertion point
    uint16_t left = note_buffer.head;
    uint16_t right = note_buffer.tail;
    uint16_t insert_pos = left;

    while (left != right) {
        // calculate mid-point
        uint16_t mid = (left + (right - left + NOTE_BUFFER_SIZE) % NOTE_BUFFER_SIZE / 2) % NOTE_BUFFER_SIZE;

        // compare timestamps: new note vs mid note
        if (new_note.us_since_1900 < note_buffer.note[mid].us_since_1900) {
            // move right pointer back to mid
            right = mid;
        } else {
            // move left pointer fwd to mid + 1
            left = (mid + 1) % NOTE_BUFFER_SIZE;
        }
    }
    insert_pos = left;  // insertion point found

    // make room for the new note
    uint16_t current_pos = note_buffer.tail;
    while (current_pos != insert_pos) {
        uint16_t prev_pos = (current_pos - 1 + NOTE_BUFFER_SIZE) % NOTE_BUFFER_SIZE;
        note_buffer.note[current_pos] = note_buffer.note[prev_pos];
        current_pos = prev_pos;
    }

    // insert new note
    note_buffer.note[insert_pos] = new_note; // copy
    note_buffer.tail = (note_buffer.tail + 1) % NOTE_BUFFER_SIZE;
    note_buffer.count++;
}

void Network_source::note_on(tlv_packet_t *tp)
{
  enqueue_note(tp, 1);
}

void Network_source::note_off(tlv_packet_t *tp)
{
  enqueue_note(tp, 0);
}

void Network_source::note_on_off(tlv_packet_t *tp)
{
  tlv_type_note_on_off_t *p = (tlv_type_note_on_off_t *)tp->payload;

  tlv_packet_t on;
  tlv_packet_t off;
  tlv_type_note_on_t *pon = (tlv_type_note_on_t *)on.payload;
  tlv_type_note_on_t *poff = (tlv_type_note_on_t *)off.payload;

  pon->note = p->note;
  poff->note = p->note;
  pon->channel = p->channel;
  poff->channel = p->channel;
  pon->velocity = p->velocity;
  poff->velocity = p->velocity;
  pon->us_since_1900 = p->on;
  poff->us_since_1900 = p->off;

  enqueue_note(&on, 1);
  enqueue_note(&off, 0);
}

void Network_source::beat(tlv_packet_t *tp)
{
  static uint32_t last_count = 0xffffffff;
  tlv_type_beat_t *p = (tlv_type_beat_t *)tp->payload;
  if (p->count != last_count + 1)
  {
    printf("WARNING: beat jumped from %ld to %ld\n", last_count, p->count);
  }
  last_count = p->count;
  printf("bpm %d, beat %ld, note_count %d\n", p->bpm, p->count, note_buffer.count);
}

void Network_source::start(tlv_packet_t *tp)
{
  tlv_type_start_t *p = (tlv_type_start_t *)tp->payload;
  printf("start will be at %llu us after 1900 with bpm %d, beat %ld\n", p->us_since_1900, p->bpm, p->count);
  _start = p->us_since_1900;
  _bpm = p->bpm;
  _beat = p->count;
}

void Network_source::panic(tlv_packet_t *tp)
{
  (void)tp;
  uint8_t packet[4];
  packet[0] = 0x08;  // note off
  packet[3] = 0;     // velocity
  for (int i=0; i < 128; i++)
  {
      packet[2] = i; // note
      for (int c = 0; c<0x10; c++)
      {
        packet[1] = c;     // channel
        _midi_state_machine->consume_event_packet(packet);
      }
  }
  printf("\n   ===   PANIC   ===\n\n");
}

void Network_source::scale(tlv_packet_t *tp)
{
   // FIXME these need to go through the FIFO too, just as notes
  tlv_type_scale_t *p = (tlv_type_scale_t *)tp->payload;
  _root = p->root;
  _scale_type = p->scale_type;

  printf("got a %s%d based %s scale\n",
          note_name[_root%12],
          _root/12-1,
          scale_name[_scale_type-1]
        );
}

void Network_source::chord_flavour0(tlv_packet_t *tp)
{
  //
  // just the plain chord notes, simultaneously
  //
  tlv_type_chord_t *p = (tlv_type_chord_t *)tp->payload;

  tlv_packet_t pp;
  pp.header.type = TLV_TYPE_NOTE_ON;
  pp.header.len = sizeof(tlv_header_t) + sizeof(tlv_type_note_on_t);
  tlv_type_note_on_t *n = (tlv_type_note_on_t *)(&pp.payload);
  n->channel = 1;
  n->velocity = 255;

  for (int i = 0; i < 16; i++)
  {
    n->note = p->note[i];
    n->us_since_1900 = p->on;
    if (n->note < 0x80) enqueue_note(&pp, 1);
    n->us_since_1900 = p->off;
    if (n->note < 0x80) enqueue_note(&pp, 0);
  }
}

void Network_source::chord_flavour1(tlv_packet_t *tp)
{
  //
  // plain chord notes, simultaneously with added bass
  // (root notes lowered down to lowest MIDI octave)
  //
  tlv_type_chord_t *p = (tlv_type_chord_t *)tp->payload;

  tlv_packet_t pp;
  pp.header.type = TLV_TYPE_NOTE_ON;
  pp.header.len = sizeof(tlv_header_t) + sizeof(tlv_type_note_on_t);
  tlv_type_note_on_t *n = (tlv_type_note_on_t *)(&pp.payload);
  n->channel = 1;
  n->velocity = 255;
  n->us_since_1900 = p->on;

  uint8_t r;
  r = p->note[0];
  if (r!=0x80)
  {
    do
    {
      n->note = r;
      r-=12;
      n->us_since_1900 = p->on;
      enqueue_note(&pp, 1);
      n->us_since_1900 = p->off;
      enqueue_note(&pp, 0);
    } while (r>12);
  }

  for (int i = 1; i < 16; i++)
  {
    n->note = p->note[i];
    n->us_since_1900 = p->on;
    if (n->note < 0x80) enqueue_note(&pp, 1);
    n->us_since_1900 = p->off;
    if (n->note < 0x80) enqueue_note(&pp, 0);
  }
}

void Network_source::chord_flavour000_b0rken(tlv_packet_t *tp)
{
  //
  // chord notes evenly spread, rising
  //
  tlv_type_chord_t *p = (tlv_type_chord_t *)tp->payload;

  tlv_packet_t pp;
  pp.header.type = TLV_TYPE_NOTE_ON;
  pp.header.len = sizeof(tlv_header_t) + sizeof(tlv_type_note_on_t);
  tlv_type_note_on_t *n = (tlv_type_note_on_t *)(&pp.payload);
  n->channel = 1;
  n->velocity = 255;

  int note_count = 0;
  for (int i = 0; i < 16; i++)
  {
    if (p->note[i] < 0x80) note_count++;
  }

  uint64_t tq = (p->off - p->on) / (note_count+1);

  for (int i = 0; i < 16; i++)
  {
    n->note = p->note[i];
    n->us_since_1900 = p->on + (i + 0) * tq;
    if (n->note < 0x80) enqueue_note(&pp, 1);
    n->us_since_1900 = p->on + (i + 1) * tq;
    if (n->note < 0x80) enqueue_note(&pp, 0);
  }
}

void Network_source::chord_flavour001_b0rken(tlv_packet_t *tp)
{
  //
  // chord notes evenly spread, falling
  //
  tlv_type_chord_t *p = (tlv_type_chord_t *)tp->payload;

  tlv_packet_t pp;
  pp.header.type = TLV_TYPE_NOTE_ON;
  pp.header.len = sizeof(tlv_header_t) + sizeof(tlv_type_note_on_t);
  tlv_type_note_on_t *n = (tlv_type_note_on_t *)(&pp.payload);
  n->channel = 1;
  n->velocity = 255;

  int note_count = 0;
  for (int i = 0; i < 16; i++)
  {
    if (p->note[i] < 0x80) note_count++;
  }

  uint64_t tq = (p->off - p->on) / (note_count+1);

  for (int i = 15; i >= 0; i--)
  {
    n->note = p->note[i];
    n->us_since_1900 = p->on + (i + 0) * tq;
    if (n->note < 0x80) enqueue_note(&pp, 1);
    n->us_since_1900 = p->on + (i + 1) * tq;
    if (n->note < 0x80) enqueue_note(&pp, 0);
  }
}

void Network_source::chord_flavour2(tlv_packet_t *tp)
{
  //
  // chord notes evenly spread, rising
  //
  tlv_type_chord_t *p = (tlv_type_chord_t *)tp->payload;

  tlv_packet_t pp;
  pp.header.type = TLV_TYPE_NOTE_ON;
  pp.header.len = sizeof(tlv_header_t) + sizeof(tlv_type_note_on_t);
  tlv_type_note_on_t *n = (tlv_type_note_on_t *)(&pp.payload);
  n->channel = 1;
  n->velocity = 255;

  int note_count = 0;
  for (int i = 0; i < 16; i++)
  {
    if (p->note[i] < 0x80) note_count++;
  }

  uint64_t tq = (p->off - p->on) / note_count; // time quantum
  int tqx = 0; // tq index

  for (int i = 0; i < 16; i++)
  {
    n->note = p->note[i];
    if (p->note[i] < 0x80)
    {
      n->us_since_1900 = p->on + tqx * tq;
      enqueue_note(&pp, 1);
      tqx++;
      n->us_since_1900 = p->on + tqx * tq;
      enqueue_note(&pp, 0);
    }
  }
}

void Network_source::chord_flavour3(tlv_packet_t *tp)
{
  //
  // chord notes evenly spread, falling
  //
  tlv_type_chord_t *p = (tlv_type_chord_t *)tp->payload;

  tlv_packet_t pp;
  pp.header.type = TLV_TYPE_NOTE_ON;
  pp.header.len = sizeof(tlv_header_t) + sizeof(tlv_type_note_on_t);
  tlv_type_note_on_t *n = (tlv_type_note_on_t *)(&pp.payload);
  n->channel = 1;
  n->velocity = 255;

  int note_count = 0;
  for (int i = 0; i < 16; i++)
  {
    if (p->note[i] < 0x80) note_count++;
  }

  uint64_t tq = (p->off - p->on) / note_count; // time quantum
  int tqx = 0; // tq index

  for (int i = 15; i >= 0; i--)
  {
    n->note = p->note[i];
    if (p->note[i] < 0x80)
    {
      n->us_since_1900 = p->on + tqx * tq;
      enqueue_note(&pp, 1);
      tqx++;
      n->us_since_1900 = p->on + tqx * tq;
      enqueue_note(&pp, 0);
    }
  }
}

void Network_source::double_chord_down(tlv_packet_t *tp)
{
  tlv_type_chord_t *p = (tlv_type_chord_t *)tp->payload;

  uint8_t shadow[16];
  memcpy(shadow, p->note, 16);

  for (int i = 0; i < 16; i++)
  {
    if (p->note[i] < 0x80)
    {
      for (int j = i; j < 16; j++)
      {
        if (shadow[j] == 0x80)
        {
          shadow[j] = p->note[i] - 12;
          break;
        }
      }
    }
  }
  memcpy(p->note, shadow, 16);
}

void Network_source::chord_flavour4(tlv_packet_t *tp)
{
  //
  // same as flavour2 just extended the chord 1 octave down
  //
  double_chord_down(tp);
  chord_flavour2(tp);
}

void Network_source::chord_flavour5(tlv_packet_t *tp)
{
  //
  // same as flavour3 just extended the chord 1 octave down
  //
  double_chord_down(tp);
  chord_flavour3(tp);
}

void Network_source::sort_notes(tlv_packet_t *tp)
{
  tlv_type_chord_t *p = (tlv_type_chord_t *)tp->payload;

  for (int i = 0; i < 16; i++)
  {
    for (int j = 0; j < 15 - i; j++)
    {
      if (p->note[j] > p->note[j + 1])
      {
        // swap
        uint8_t temp = p->note[j];
        p->note[j] = p->note[j + 1];
        p->note[j + 1] = temp;
      }
    }
  }
}

void Network_source::chord_flavour6(tlv_packet_t *tp)
{
  //
  // same as flavour2 just extended the chord 1 octave down, sort
  //
  double_chord_down(tp);
  sort_notes(tp);
  chord_flavour2(tp);
}

void Network_source::chord_flavour7(tlv_packet_t *tp)
{
  //
  // same as flavour3 just extended the chord 1 octave down, sort
  //
  double_chord_down(tp);
  sort_notes(tp);
  chord_flavour3(tp);
}

void Network_source::sort_notes_out2in(tlv_packet_t *tp)
{
  tlv_type_chord_t *p = (tlv_type_chord_t *)tp->payload;

  sort_notes(tp);

  uint8_t temp[16];
  int low = 0;
  int high = 15;

  for (int i = 0; i < 16; i++)
  {
    if (i % 2 == 0)
    {
      temp[i] = p->note[low++];
    }
    else
    {
      temp[i] = p->note[high--];
    }
  }

  for (int i = 0; i < 16; i++)
  {
    p->note[i] = temp[i];
  }
}

void Network_source::chord_flavour8(tlv_packet_t *tp)
{
  //
  // single notes, from out to in
  //
  double_chord_down(tp);
  sort_notes_out2in(tp);
  chord_flavour2(tp); // just enq's them sequentially
}

void Network_source::chord_flavour9(tlv_packet_t *tp)
{
  //
  // single notes, from in to out
  //
  double_chord_down(tp);
  sort_notes_out2in(tp);
  chord_flavour3(tp); // just enq's them reverse sequentially
}

// RGB LED colors
uint8_t col[10][3] = {
    {0xd0, 0x60, 0x60}, // soft red
    {0x60, 0xd0, 0x60}, // soft green
    {0x80, 0x80, 0xc0}, // soft blue
    {0xff, 0xbb, 0x77}, // peach
    {0xcc, 0x77, 0xff}, // lavender
    {0x77, 0xff, 0xbb}, // mint
    {0xff, 0xd0, 0x60}, // pastel yellow
    {0xb0, 0xff, 0xe0}, // pale turquoise
    {0xff, 0xaa, 0xaa}, // soft pink
    {0x77, 0xcc, 0xff}  // sky blue
  };

void Network_source::chord(tlv_packet_t *tp)
{
  tlv_type_chord_t *p = (tlv_type_chord_t *)tp->payload;

  _root = p->note[0];
  printf("got a %s%d based chord\n",
          note_name[_root%12],
          _root/12-1);

  int choice = rand() % 10;
   // FIXME led colors need to go through the FIFO too, just as notes
  set_first_led(col[choice][0], col[choice][1], col[choice][2]);
  update_leds();
  switch (choice)
  {
    case 0:
      chord_flavour0(tp);
      break;
    case 1:
      chord_flavour1(tp);
      break;
    case 2:
      chord_flavour2(tp);
      break;
    case 3:
      chord_flavour3(tp);
      break;
    case 4:
      chord_flavour4(tp);
      break;
    case 5:
      chord_flavour5(tp);
      break;
    case 6:
      chord_flavour6(tp);
      break;
    case 7:
      chord_flavour7(tp);
      break;
    case 8:
      chord_flavour8(tp);
      break;
    case 9:
      chord_flavour9(tp);
      break;
  }
}

void Network_source::artist(tlv_packet_t *tp)
{
  tlv_type_artist_t *p = (tlv_type_artist_t *)tp->payload;
  memcpy(_artist, p->artist, 234);
  _artist[234] = 0;
  printf("artist: %s\n", _artist);
}

void Network_source::title(tlv_packet_t *tp)
{
  tlv_type_title_t *p = (tlv_type_title_t *)tp->payload;
  memcpy(_title, p->title, 234);
  _title[234] = 0;
  printf("title: %s\n", _title);
}

void Network_source::init_tlv(TLV_registry& registry) {
    registry.set_callback(TLV_TYPE_TIME, [this](tlv_packet_t *p) { this->tlv_time(p); });
    registry.set_callback(TLV_TYPE_NOTE_ON, [this](tlv_packet_t *p) { this->note_on(p); });
    registry.set_callback(TLV_TYPE_NOTE_OFF, [this](tlv_packet_t *p) { this->note_off(p); });
    registry.set_callback(TLV_TYPE_NOTE_ON_OFF, [this](tlv_packet_t *p) { this->note_on_off(p); });
    registry.set_callback(TLV_TYPE_BEAT, [this](tlv_packet_t *p) { this->beat(p); });
    registry.set_callback(TLV_TYPE_START, [this](tlv_packet_t *p) { this->start(p); });
    registry.set_callback(TLV_TYPE_PANIC, [this](tlv_packet_t *p) { this->panic(p); });
    registry.set_callback(TLV_TYPE_SCALE, [this](tlv_packet_t *p) { this->scale(p); });
    registry.set_callback(TLV_TYPE_CHORD, [this](tlv_packet_t *p) { this->chord(p); });
    registry.set_callback(TLV_TYPE_ARTIST, [this](tlv_packet_t *p) { this->artist(p); });
    registry.set_callback(TLV_TYPE_TITLE, [this](tlv_packet_t *p) { this->title(p); });
}
