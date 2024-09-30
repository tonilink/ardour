/*
 * Copyright (C) 2005-2007 Doug McLain <doug@nostar.net>
 * Copyright (C) 2005-2017 Tim Mayberry <mojofunk@gmail.com>
 * Copyright (C) 2005-2019 Paul Davis <paul@linuxaudiosystems.com>
 * Copyright (C) 2005 Karsten Wiese <fzuuzf@googlemail.com>
 * Copyright (C) 2005 Taybin Rutkin <taybin@taybin.com>
 * Copyright (C) 2006-2015 David Robillard <d@drobilla.net>
 * Copyright (C) 2007-2012 Carl Hetherington <carl@carlh.net>
 * Copyright (C) 2008-2010 Sakari Bergen <sakari.bergen@beatwaves.net>
 * Copyright (C) 2012-2019 Robin Gareus <robin@gareus.org>
 * Copyright (C) 2013-2015 Colin Fletcher <colin.m.fletcher@googlemail.com>
 * Copyright (C) 2013-2016 John Emmas <john@creativepost.co.uk>
 * Copyright (C) 2013-2016 Nick Mainsbridge <mainsbridge@gmail.com>
 * Copyright (C) 2014-2024 Ben Loftis <ben@harrisonconsoles.com>
 * Copyright (C) 2015 André Nusser <andre.nusser@googlemail.com>
 * Copyright (C) 2016-2018 Len Ovens <len@ovenwerks.net>
 * Copyright (C) 2017 Johannes Mueller <github@johannes-mueller.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#ifdef WAF_BUILD
#include "gtk2ardour-config.h"
#include "gtk2ardour-version.h"
#endif

#include <algorithm>
#include <cmath>
#include <iostream>

#include <stdarg.h>

#ifdef __FreeBSD__
#include <sys/types.h>
#include <sys/sysctl.h>
#endif

#include <stdint.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>

#include <glib.h>
#include "pbd/gstdio_compat.h"

#include <gtkmm/accelmap.h>
#include <gtkmm/messagedialog.h>
#include <gtkmm/stock.h>
#include <gtkmm/uimanager.h>

#include "pbd/error.h"
#include "pbd/compose.h"
#include "pbd/convert.h"
#include "pbd/failed_constructor.h"
#include "pbd/memento_command.h"
#include "pbd/openuri.h"
#include "pbd/types_convert.h"
#include "pbd/file_utils.h"
#include <pbd/localtime_r.h>
#include "pbd/pthread_utils.h"
#include "pbd/replace_all.h"
#include "pbd/scoped_file_descriptor.h"
#include "pbd/xml++.h"

#include "gtkmm2ext/application.h"
#include "gtkmm2ext/bindings.h"
#include "gtkmm2ext/gtk_ui.h"
#include "gtkmm2ext/menu_elems.h"
#include "gtkmm2ext/utils.h"
#include "gtkmm2ext/window_title.h"

#include "widgets/fastmeter.h"
#include "widgets/prompter.h"
#include "widgets/tooltips.h"

#include "ardour/ardour.h"
#include "ardour/audioengine.h"
#include "ardour/meter.h"
#include "ardour/monitor_control.h"
#include "ardour/profile.h"
#include "ardour/revision.h"
#include "ardour/transport_master.h"
#include "ardour/transport_master_manager.h"
#include "ardour/triggerbox.h"
#include "ardour/track.h"
#include "ardour/vca_manager.h"
#include "ardour/utils.h"

#include "LuaBridge/LuaBridge.h"

#ifdef WINDOWS_VST_SUPPORT
#include <fst.h>
#endif
#ifdef AUDIOUNIT_SUPPORT
#include "ardour/audio_unit.h"
#endif

// fix for OSX (nsm.h has a check function, AU/Apple defines check)
#ifdef check
#undef check
#endif

#include "temporal/time.h"

#include "control_protocol/basic_ui.h"

#include "about.h"
#include "actions.h"
#include "add_route_dialog.h"
#include "ardour_message.h"
#include "ardour_ui.h"
#include "audio_clock.h"
#include "audio_region_view.h"
#include "big_clock_window.h"
#include "big_transport_window.h"
#include "bundle_manager.h"
#include "dsp_stats_window.h"
#include "duplicate_routes_dialog.h"
#include "debug.h"
#include "engine_dialog.h"
#include "export_video_dialog.h"
#include "global_port_matrix.h"
#include "gui_object.h"
#include "gui_thread.h"
#include "idleometer.h"
#include "instrument_selector.h"
#include "io_plugin_window.h"
#include "keyboard.h"
#include "keyeditor.h"
#include "library_download_dialog.h"
#include "location_ui.h"
#include "lua_script_manager.h"
#include "luawindow.h"
#include "main_clock.h"
#include "missing_plugin_dialog.h"
#include "mixer_ui.h"
#include "meterbridge.h"
#include "mouse_cursors.h"
#include "nsm.h"
#include "opts.h"
#include "plugin_dspload_window.h"
#include "plugin_manager_ui.h"
#include "processor_box.h"
#include "public_editor.h"
#include "rc_option_editor.h"
#include "recorder_ui.h"
#include "route_time_axis.h"
#include "route_params_ui.h"
#include "save_as_dialog.h"
#include "save_template_dialog.h"
#include "script_selector.h"
#include "session_archive_dialog.h"
#include "session_dialog.h"
#include "session_metadata_dialog.h"
#include "session_option_editor.h"
#include "speaker_dialog.h"
#include "splash.h"
#include "template_dialog.h"
#include "time_axis_view_item.h"
#include "time_info_box.h"
#include "timers.h"
#include "transport_masters_dialog.h"
#include "trigger_page.h"
#include "triggerbox_ui.h"
#include "utils.h"
#include "virtual_keyboard_window.h"
#include "add_video_dialog.h"
#include "plugin_selector.h"

#include "pbd/i18n.h"

#include "transport_bar.h"

using namespace ARDOUR;
using namespace ARDOUR_UI_UTILS;
using namespace PBD;
using namespace Gtkmm2ext;
using namespace ArdourWidgets;
using namespace Gtk;
using namespace std;
using namespace Editing;
using namespace Gtk::Menu_Helpers;

static const gchar *_record_mode_strings[] = {
	N_("Layered"),
	N_("Non-Layered"),
	N_("Snd on Snd"),
	0
};

#define PX_SCALE(px) std::max((float)px, rintf((float)px * UIConfiguration::instance().get_ui_scale()))

TransportBar::TransportBar ()
	: _basic_ui (0)
	, latency_disable_button (ArdourButton::led_default_elements)
	, follow_edits_button (ArdourButton::led_default_elements)
	, auto_return_button (ArdourButton::led_default_elements)
	, primary_clock  (X_("primary"),   X_("transport"), MainClock::PrimaryClock)
	, secondary_clock (X_("secondary"), X_("secondary"), MainClock::SecondaryClock)
	, secondary_clock_spacer (0)
	, auditioning_alert_button (_("Audition"))
	, solo_alert_button (_("Solo"))
	, feedback_alert_button (_("Feedback"))
	, _feedback_exists (false)
{
	record_mode_strings = I18N (_record_mode_strings);

	transport_ctrl.setup (ARDOUR_UI::instance ());
	transport_ctrl.map_actions ();

	/* sync_button */
	Glib::RefPtr<Action> act = ActionManager::get_action (X_("Transport"), X_("ToggleExternalSync"));
	sync_button.set_related_action (act);
	sync_button.signal_button_press_event().connect (sigc::mem_fun (*this, &TransportBar::sync_button_clicked), false);
	sync_button.set_sizing_text (S_("LogestSync|M-Clk"));

	/* sub-layout for Sync | Shuttle (grow) */
	HBox* ssbox = manage (new HBox);
	ssbox->set_spacing (PX_SCALE(2));
	ssbox->pack_start (sync_button, false, false, 0);
	ssbox->pack_start (shuttle_box, true, true, 0);
	ssbox->pack_start (*shuttle_box.vari_button(), false, false, 0);
	ssbox->pack_start (*shuttle_box.info_button(), false, false, 0);

	punch_label.set_text (_("Punch:"));
	layered_label.set_text (_("Rec:"));

	punch_in_button.set_text (S_("Punch|In"));
	punch_out_button.set_text (S_("Punch|Out"));

	record_mode_selector.AddMenuElem (MenuElem (record_mode_strings[(int)RecLayered], sigc::bind (sigc::mem_fun (*this, &TransportBar::set_record_mode), RecLayered)));
	record_mode_selector.AddMenuElem (MenuElem (record_mode_strings[(int)RecNonLayered], sigc::bind (sigc::mem_fun (*this, &TransportBar::set_record_mode), RecNonLayered)));
	record_mode_selector.AddMenuElem (MenuElem (record_mode_strings[(int)RecSoundOnSound], sigc::bind (sigc::mem_fun (*this, &TransportBar::set_record_mode), RecSoundOnSound)));
	record_mode_selector.set_sizing_texts (record_mode_strings);

	act = ActionManager::get_action ("Transport", "TogglePunchIn");
	punch_in_button.set_related_action (act);
	act = ActionManager::get_action ("Transport", "TogglePunchOut");
	punch_out_button.set_related_action (act);

	act = ActionManager::get_action ("Main", "ToggleLatencyCompensation");
	latency_disable_button.set_related_action (act);

	latency_disable_button.set_text (_("Disable PDC"));
	io_latency_label.set_text (_("I/O Latency:"));

	set_size_request_to_display_given_text (route_latency_value, "1000 spl", 0, 0);
	set_size_request_to_display_given_text (io_latency_value, "888.88 ms", 0, 0);

	act = ActionManager::get_action ("Transport", "ToggleAutoReturn");
	auto_return_button.set_related_action (act);
	act = ActionManager::get_action (X_("Transport"), X_("ToggleFollowEdits"));
	follow_edits_button.set_related_action (act);

	auto_return_button.set_text(_("Auto Return"));
	follow_edits_button.set_text(_("Follow Range"));

	/* CANNOT sigc::bind these to clicked or toggled, must use pressed or released */
	act = ActionManager::get_action (X_("Main"), X_("cancel-solo"));
	solo_alert_button.set_related_action (act);
	auditioning_alert_button.signal_clicked.connect (sigc::mem_fun(*this,&TransportBar::audition_alert_clicked));

	/* alert box sub-group */
	VBox* alert_box = manage (new VBox);
	alert_box->set_homogeneous (true);
	alert_box->set_spacing (1);
	alert_box->set_border_width (0);
	alert_box->pack_start (solo_alert_button, true, true);
	alert_box->pack_start (auditioning_alert_button, true, true);
	alert_box->pack_start (feedback_alert_button, true, true);

	/* monitor section sub-group */
	VBox* monitor_box = manage (new VBox);
	monitor_box->set_homogeneous (true);
	monitor_box->set_spacing (1);
	monitor_box->set_border_width (0);
	monitor_box->pack_start (monitor_mono_button, true, true);
	monitor_box->pack_start (monitor_dim_button, true, true);
	monitor_box->pack_start (monitor_mute_button, true, true);

	act = ActionManager::get_action (X_("Monitor Section"), X_("monitor-dim-all"));
	monitor_dim_button.set_related_action (act);
	act = ActionManager::get_action (X_("Monitor Section"), X_("monitor-mono"));
	monitor_mono_button.set_related_action (act);
	act = ActionManager::get_action (X_("Monitor Section"), X_("monitor-cut-all"));
	monitor_mute_button.set_related_action (act);

	monitor_dim_button.set_text (_("Dim All"));
	monitor_mono_button.set_text (_("Mono"));
	monitor_mute_button.set_text (_("Mute All"));

	int vpadding = 1;
	int hpadding = 2;
	int col = 0;
#define TCOL col, col + 1

	transport_table.attach (transport_ctrl, TCOL, 0, 1 , SHRINK, SHRINK, 0, 0);
	transport_table.attach (*ssbox,         TCOL, 1, 2 , FILL,   SHRINK, 0, 0);
	++col;

	transport_table.attach (*(manage (new ArdourVSpacer ())), TCOL, 0, 2 , SHRINK, EXPAND|FILL, 3, 0);
	++col;

	transport_table.attach (punch_label, TCOL, 0, 1 , FILL, SHRINK, 3, 0);
	transport_table.attach (layered_label, TCOL, 1, 2 , FILL, SHRINK, 3, 0);
	++col;

	transport_table.attach (punch_in_button,      col,      col + 1, 0, 1 , FILL, SHRINK, hpadding, vpadding);
	transport_table.attach (punch_space,          col + 1,  col + 2, 0, 1 , FILL, SHRINK, 0, vpadding);
	transport_table.attach (punch_out_button,     col + 2,  col + 3, 0, 1 , FILL, SHRINK, hpadding, vpadding);
	transport_table.attach (record_mode_selector, col,      col + 3, 1, 2 , FILL, SHRINK, hpadding, vpadding);
	col += 3;

	transport_table.attach (recpunch_spacer, TCOL, 0, 2 , SHRINK, EXPAND|FILL, 3, 0);
	++col;

	transport_table.attach (latency_disable_button, TCOL, 0, 1 , FILL, SHRINK, hpadding, vpadding);
	transport_table.attach (io_latency_label, TCOL, 1, 2 , SHRINK, EXPAND|FILL, hpadding, 0);
	++col;
	transport_table.attach (route_latency_value, TCOL, 0, 1 , SHRINK, EXPAND|FILL, hpadding, 0);
	transport_table.attach (io_latency_value, TCOL, 1, 2 , SHRINK, EXPAND|FILL, hpadding, 0);
	++col;

	route_latency_value.set_alignment (Gtk::ALIGN_END, Gtk::ALIGN_CENTER);
	io_latency_value.set_alignment (Gtk::ALIGN_END, Gtk::ALIGN_CENTER);

	transport_table.attach (latency_spacer, TCOL, 0, 2 , SHRINK, EXPAND|FILL, 3, 0);
	++col;

	transport_table.attach (follow_edits_button, TCOL, 0, 1 , FILL, SHRINK, hpadding, vpadding);
	transport_table.attach (auto_return_button,  TCOL, 1, 2 , FILL, SHRINK, hpadding, vpadding);
	++col;

	transport_table.attach (*(manage (new ArdourVSpacer ())), TCOL, 0, 2 , SHRINK, EXPAND|FILL, 3, 0);
	++col;

	transport_table.attach (primary_clock,                col,     col + 2, 0, 1 , FILL, SHRINK, hpadding, 0);
	transport_table.attach (*(primary_clock.left_btn()),  col,     col + 1, 1, 2 , FILL, SHRINK, hpadding, 0);
	transport_table.attach (*(primary_clock.right_btn()), col + 1, col + 2, 1, 2 , FILL, SHRINK, hpadding, 0);
	col += 2;

	transport_table.attach (*(manage (new ArdourVSpacer ())), TCOL, 0, 2 , SHRINK, EXPAND|FILL, 3, 0);
	++col;

	if (!ARDOUR::Profile->get_small_screen()) {
		transport_table.attach (secondary_clock,                col,     col + 2, 0, 1 , FILL, SHRINK, hpadding, 0);
		transport_table.attach (*(secondary_clock.left_btn()),  col,     col + 1, 1, 2 , FILL, SHRINK, hpadding, 0);
		transport_table.attach (*(secondary_clock.right_btn()), col + 1, col + 2, 1, 2 , FILL, SHRINK, hpadding, 0);
		(ARDOUR_UI::instance()->secondary_clock)->set_no_show_all (true);
		(ARDOUR_UI::instance()->secondary_clock)->left_btn()->set_no_show_all (true);
		(ARDOUR_UI::instance()->secondary_clock)->right_btn()->set_no_show_all (true);
		col += 2;

		secondary_clock_spacer = manage (new ArdourVSpacer ());
		transport_table.attach (*secondary_clock_spacer, TCOL, 0, 2 , SHRINK, EXPAND|FILL, 3, 0);
		++col;
	}

	transport_table.attach (*alert_box, TCOL, 0, 2, SHRINK, EXPAND|FILL, hpadding, 0);
	++col;

	transport_table.attach (monitor_spacer, TCOL, 0, 2 , SHRINK, EXPAND|FILL, 3, 0);
	++col;

	transport_table.attach (*monitor_box, TCOL, 0, 2 , SHRINK, EXPAND|FILL, 3, 0);
	++col;


	transport_table.set_spacings (0);
	transport_table.set_row_spacings (4);
	transport_table.set_border_width (1);
	transport_table.show_all();  //TODO: update visibility somewhere else
	pack_start(transport_table, false, false);

	/*sizing */
	Glib::RefPtr<SizeGroup> button_height_size_group = SizeGroup::create (Gtk::SIZE_GROUP_VERTICAL);
	button_height_size_group->add_widget (sync_button);
	button_height_size_group->add_widget (punch_in_button);
	button_height_size_group->add_widget (punch_out_button);
	button_height_size_group->add_widget (record_mode_selector);
	button_height_size_group->add_widget (latency_disable_button);
	button_height_size_group->add_widget (follow_edits_button);
	button_height_size_group->add_widget (auto_return_button);

	/* clock button size groups */
	button_height_size_group->add_widget (*primary_clock.left_btn());
	button_height_size_group->add_widget (*primary_clock.right_btn());
	button_height_size_group->add_widget (*secondary_clock.left_btn());
	button_height_size_group->add_widget (*secondary_clock.right_btn());

	Glib::RefPtr<SizeGroup> punch_button_size_group = SizeGroup::create (Gtk::SIZE_GROUP_HORIZONTAL);
	punch_button_size_group->add_widget (punch_in_button);
	punch_button_size_group->add_widget (punch_out_button);

	Glib::RefPtr<SizeGroup> clock1_size_group = SizeGroup::create (SIZE_GROUP_HORIZONTAL);
	clock1_size_group->add_widget (*primary_clock.left_btn());
	clock1_size_group->add_widget (*primary_clock.right_btn());

	Glib::RefPtr<SizeGroup> clock2_size_group = SizeGroup::create (SIZE_GROUP_HORIZONTAL);
	clock2_size_group->add_widget (*secondary_clock.left_btn());
	clock2_size_group->add_widget (*secondary_clock.right_btn());

	Glib::RefPtr<SizeGroup> monitor_button_size_group = SizeGroup::create (Gtk::SIZE_GROUP_HORIZONTAL);
	monitor_button_size_group->add_widget (monitor_dim_button);
	monitor_button_size_group->add_widget (monitor_mono_button);
	monitor_button_size_group->add_widget (monitor_mute_button);

	/* tooltips */
	Gtkmm2ext::UI::instance()->set_tip (punch_in_button, _("Start recording at auto-punch start"));
	Gtkmm2ext::UI::instance()->set_tip (punch_out_button, _("Stop recording at auto-punch end"));
	Gtkmm2ext::UI::instance()->set_tip (record_mode_selector, _("<b>Layered</b>, new recordings will be added as regions on a layer atop existing regions.\n<b>SoundOnSound</b>, behaves like <i>Layered</i>, except underlying regions will be audible.\n<b>Non Layered</b>, the underlying region will be spliced and replaced with the newly recorded region."));
	Gtkmm2ext::UI::instance()->set_tip (latency_disable_button, _("Disable all Plugin Delay Compensation. This results in the shortest delay from live input to output, but any paths with delay-causing plugins will sound later than those without."));
	Gtkmm2ext::UI::instance()->set_tip (auto_return_button, _("Return to last playback start when stopped"));
	Gtkmm2ext::UI::instance()->set_tip (follow_edits_button, _("Playhead follows Range tool clicks, and Range selections"));
	Gtkmm2ext::UI::instance()->set_tip (primary_clock, _("<b>Primary Clock</b> right-click to set display mode. Click to edit, click+drag a digit or mouse-over+scroll wheel to modify.\nText edits: right-to-left overwrite <tt>Esc</tt>: cancel; <tt>Enter</tt>: confirm; postfix the edit with '+' or '-' to enter delta times.\n"));
	Gtkmm2ext::UI::instance()->set_tip (secondary_clock, _("<b>Secondary Clock</b> right-click to set display mode. Click to edit, click+drag a digit or mouse-over+scroll wheel to modify.\nText edits: right-to-left overwrite <tt>Esc</tt>: cancel; <tt>Enter</tt>: confirm; postfix the edit with '+' or '-' to enter delta times.\n"));
	Gtkmm2ext::UI::instance()->set_tip (solo_alert_button, _("When active, something is soloed.\nClick to de-solo everything"));
	Gtkmm2ext::UI::instance()->set_tip (auditioning_alert_button, _("When active, auditioning is taking place.\nClick to stop the audition"));
	Gtkmm2ext::UI::instance()->set_tip (feedback_alert_button, _("When lit, there is a ports connection issue, leading to feedback loop or ambiguous alignment.\nThis is caused by connecting an output back to some input (feedback), or by multiple connections from a source to the same output via different paths (ambiguous latency, record alignment)."));
	Gtkmm2ext::UI::instance()->set_tip (monitor_dim_button, _("Monitor section dim output"));
	Gtkmm2ext::UI::instance()->set_tip (monitor_mono_button, _("Monitor section mono output"));
	Gtkmm2ext::UI::instance()->set_tip (monitor_mute_button, _("Monitor section mute output"));


	/* theme-ing */
	sync_button.set_name ("transport active option button");
	punch_in_button.set_name ("punch button");
	punch_out_button.set_name ("punch button");
	record_mode_selector.set_name ("record mode button");
	latency_disable_button.set_name ("latency button");
	auto_return_button.set_name ("transport option button");
	follow_edits_button.set_name ("transport option button");
	solo_alert_button.set_name ("rude solo");
	auditioning_alert_button.set_name ("rude audition");
	feedback_alert_button.set_name ("feedback alert");
	monitor_dim_button.set_name ("monitor section dim");
	monitor_mono_button.set_name ("monitor section mono");
	monitor_mute_button.set_name ("mute button");

	monitor_dim_button.set_layout_font (UIConfiguration::instance().get_SmallerFont());
	monitor_mono_button.set_layout_font (UIConfiguration::instance().get_SmallerFont());
	monitor_mute_button.set_layout_font (UIConfiguration::instance().get_SmallerFont());

	monitor_dim_button.set_elements (ArdourButton::Element(ArdourButton::Body|ArdourButton::Text));
	monitor_mono_button.set_elements (ArdourButton::Element(ArdourButton::Body|ArdourButton::Text));
	monitor_mute_button.set_elements (ArdourButton::Element(ArdourButton::Body|ArdourButton::Text));

	solo_alert_button.set_elements (ArdourButton::Element(ArdourButton::Body|ArdourButton::Text));
	auditioning_alert_button.set_elements (ArdourButton::Element(ArdourButton::Body|ArdourButton::Text));
	feedback_alert_button.set_elements (ArdourButton::Element(ArdourButton::Body|ArdourButton::Text));

	solo_alert_button.set_layout_font (UIConfiguration::instance().get_SmallerFont());
	auditioning_alert_button.set_layout_font (UIConfiguration::instance().get_SmallerFont());
	feedback_alert_button.set_layout_font (UIConfiguration::instance().get_SmallerFont());

	feedback_alert_button.set_sizing_text (_("Facdbeek")); //< longest of "Feedback" and "No Align"


	/* indicate global latency compensation en/disable */
	ARDOUR::Latent::DisableSwitchChanged.connect (forever_connections, MISSING_INVALIDATOR, boost::bind (&TransportBar::latency_switch_changed, this), gui_context ());
	ARDOUR::Session::FeedbackDetected.connect (forever_connections, MISSING_INVALIDATOR, boost::bind (&TransportBar::feedback_detected, this), gui_context ());
	ARDOUR::Session::SuccessfulGraphSort.connect (forever_connections, MISSING_INVALIDATOR, boost::bind (&TransportBar::successful_graph_sort, this), gui_context ());

	/*initialize */
	repack_transport_hbox ();
	update_clock_visibility ();
	set_transport_sensitivity (false);
	latency_switch_changed ();
	session_latency_updated (true);

	/* desensitize */
	feedback_alert_button.set_sensitive (false);
	feedback_alert_button.set_visual_state (Gtkmm2ext::NoVisualState);
	auditioning_alert_button.set_sensitive (false);
	auditioning_alert_button.set_visual_state (Gtkmm2ext::NoVisualState);
}
#undef PX_SCALE
#undef TCOL

TransportBar::~TransportBar ()
{
}


void
TransportBar::repack_transport_hbox ()
{
/*	if (time_info_box) {
		if (time_info_box->get_parent()) {
			transport_hbox.remove (*time_info_box);
		}
		if (UIConfiguration::instance().get_show_toolbar_selclock ()) {
			transport_hbox.pack_start (*time_info_box, false, false);
			time_info_box->show();
		}
	}

	if (mini_timeline.get_parent()) {
		transport_hbox.remove (mini_timeline);
	}
	if (UIConfiguration::instance().get_show_mini_timeline ()) {
		transport_hbox.pack_start (mini_timeline, true, true);
		mini_timeline.show();
	}

	if (editor_meter) {
		if (editor_meter_table.get_parent()) {
			transport_hbox.remove (editor_meter_table);
		}
		if (meterbox_spacer.get_parent()) {
			transport_hbox.remove (meterbox_spacer);
			transport_hbox.remove (meterbox_spacer2);
		}

		if (UIConfiguration::instance().get_show_editor_meter()) {
			transport_hbox.pack_end (meterbox_spacer, false, false, 3);
			transport_hbox.pack_end (editor_meter_table, false, false);
			transport_hbox.pack_end (meterbox_spacer2, false, false, 1);
			meterbox_spacer2.set_size_request (1, -1);
			editor_meter_table.show();
			meterbox_spacer.show();
			meterbox_spacer2.show();
		}
	}
*/
	bool show_rec = UIConfiguration::instance().get_show_toolbar_recpunch ();
	if (show_rec) {
		punch_label.show ();
		layered_label.show ();
//		punch_in_button.show ();
//		punch_out_button.show ();
		record_mode_selector.show ();
//		recpunch_spacer.show ();
	} else {
		punch_label.hide ();
		layered_label.hide ();
//		punch_in_button.hide ();
//		punch_out_button.hide ();
		record_mode_selector.hide ();
//		recpunch_spacer.hide ();
	}

/*
	bool show_pdc = UIConfiguration::instance().get_show_toolbar_latency ();
	if (show_pdc) {
		latency_disable_button.show ();
		route_latency_value.show ();
		io_latency_label.show ();
		io_latency_value.show ();
		latency_spacer.show ();
	} else {
		latency_disable_button.hide ();
		route_latency_value.hide ();
		io_latency_label.hide ();
		io_latency_value.hide ();
		latency_spacer.hide ();
	}

	bool show_cue = UIConfiguration::instance().get_show_toolbar_cuectrl ();
	if (show_cue) {
		_cue_rec_enable.show ();
		_cue_play_enable.show ();
		cuectrl_spacer.show ();
	} else {
		_cue_rec_enable.hide ();
		_cue_play_enable.hide ();
		cuectrl_spacer.hide ();
	}

	bool show_mnfo = UIConfiguration::instance().get_show_toolbar_monitor_info ();
	if (show_mnfo) {
		monitor_dim_button.show ();
		monitor_mono_button.show ();
		monitor_mute_button.show ();
		monitor_spacer.show ();
	} else {
		monitor_dim_button.hide ();
		monitor_mono_button.hide ();
		monitor_mute_button.hide ();
		monitor_spacer.hide ();
	}
*/
}

void
TransportBar::feedback_detected ()
{
	_feedback_exists = true;
}

void
TransportBar::successful_graph_sort ()
{
	_feedback_exists = false;
}

void
TransportBar::soloing_changed (bool onoff)
{
	if (solo_alert_button.get_active() != onoff) {
		solo_alert_button.set_active (onoff);
	}
}

void
TransportBar::_auditioning_changed (bool onoff)
{
	auditioning_alert_button.set_active (onoff);
	auditioning_alert_button.set_sensitive (onoff);
	if (!onoff) {
		auditioning_alert_button.set_visual_state (Gtkmm2ext::NoVisualState);
	}
	set_transport_sensitivity (!onoff);
}

void
TransportBar::auditioning_changed (bool onoff)
{
	UI::instance()->call_slot (MISSING_INVALIDATOR, boost::bind (&TransportBar::_auditioning_changed, this, onoff));
}

void
TransportBar::audition_alert_clicked ()
{
	if (_session) {
		_session->cancel_audition();
	}
}

void
TransportBar::solo_blink (bool onoff)
{
	if (_session == 0) {
		return;
	}

	if (_session->soloing() || _session->listening()) {
		if (onoff) {
			solo_alert_button.set_active (true);
		} else {
			solo_alert_button.set_active (false);
		}
	} else {
		solo_alert_button.set_active (false);
	}
}

void
TransportBar::audition_blink (bool onoff)
{
	if (_session == 0) {
		return;
	}

	if (_session->is_auditioning()) {
		if (onoff) {
			auditioning_alert_button.set_active (true);
		} else {
			auditioning_alert_button.set_active (false);
		}
	} else {
		auditioning_alert_button.set_active (false);
	}
}

void
TransportBar::feedback_blink (bool onoff)
{
	if (_feedback_exists) {
		feedback_alert_button.set_active (true);
		feedback_alert_button.set_text (_("Feedback"));
		if (onoff) {
			feedback_alert_button.reset_fixed_colors ();
		} else {
			feedback_alert_button.set_active_color (UIConfigurationBase::instance().color ("feedback alert: alt active", NULL));
		}
	} else if (_ambiguous_latency && !UIConfiguration::instance().get_show_toolbar_latency ()) {
		feedback_alert_button.set_text (_("No Align"));
		feedback_alert_button.set_active (true);
		if (onoff) {
			feedback_alert_button.reset_fixed_colors ();
		} else {
			feedback_alert_button.set_active_color (UIConfigurationBase::instance().color ("feedback alert: alt active", NULL));
		}
	} else {
		feedback_alert_button.set_text (_("Feedback"));
		feedback_alert_button.reset_fixed_colors ();
		feedback_alert_button.set_active (false);
	}
}

void
TransportBar::set_session (Session *s)
{
	SessionHandlePtr::set_session (s);

	transport_ctrl.set_session (s);
	shuttle_box.set_session (s);
	primary_clock.set_session (s);
	secondary_clock.set_session (s);

	if (_basic_ui) {
		delete _basic_ui;
	}
	_basic_ui = new BasicUI (*s);

	map_transport_state ();

	if (!_session) {
		blink_connection.disconnect ();

		return;
	}

	_session->AuditionActive.connect (_session_connections, MISSING_INVALIDATOR, boost::bind (&TransportBar::auditioning_changed, this, _1), gui_context());
	_session->TransportStateChange.connect (_session_connections, MISSING_INVALIDATOR, boost::bind (&TransportBar::map_transport_state, this), gui_context());
	_session->config.ParameterChanged.connect (_session_connections, MISSING_INVALIDATOR, boost::bind (&TransportBar::parameter_changed, this, _1), gui_context());
	_session->LatencyUpdated.connect (_session_connections, MISSING_INVALIDATOR, boost::bind (&TransportBar::session_latency_updated, this, _1), gui_context());
	_session->SoloActive.connect (_session_connections, MISSING_INVALIDATOR, boost::bind (&TransportBar::soloing_changed, this, _1), gui_context());
	_session->AuditionActive.connect (_session_connections, MISSING_INVALIDATOR, boost::bind (&TransportBar::auditioning_changed, this, _1), gui_context());

	//initialize all session config settings
	boost::function<void (std::string)> pc (boost::bind (&TransportBar::parameter_changed, this, _1));
	_session->config.map_parameters (pc);

	/* initialize */
	session_latency_updated (true);

	solo_alert_button.set_active (_session->soloing());

	blink_connection = Timers::blink_connect (sigc::mem_fun(*this, &TransportBar::blink_handler));

/*  TODO: separate actions for the Edit, Rec, Cue and Mix pages, to focus the appropriate clock (?)
	act = ActionManager::register_action (transport_actions, X_("focus-on-clock"), _("Focus On Clock"), sigc::mem_fun(*this, &TransportBar::focus_on_clock));
	ActionManager::session_sensitive_actions.push_back (act);
	ActionManager::transport_sensitive_actions.push_back (act);
*/
}

void
TransportBar::set_transport_sensitivity (bool yn)
{
	shuttle_box.set_sensitive (yn);
}

void
TransportBar::latency_switch_changed ()
{
	bool pdc_off = ARDOUR::Latent::zero_latency ();
	if (latency_disable_button.get_active() != pdc_off) {
		latency_disable_button.set_active (pdc_off);
	}
}

void
TransportBar::focus_on_clock ()
{
	primary_clock.focus ();
}

void
TransportBar::update_clock_visibility ()
{
	if (ARDOUR::Profile->get_small_screen()) {
		return;
	}
	if (UIConfiguration::instance().get_show_secondary_clock ()) {
		secondary_clock.show();
		secondary_clock.left_btn()->show();
		secondary_clock.right_btn()->show();
	} else {
		secondary_clock.hide();
		secondary_clock.left_btn()->hide();
		secondary_clock.right_btn()->hide();
	}
}

void
TransportBar::session_latency_updated (bool for_playback)
{
	if (!for_playback) {
		/* latency updates happen in pairs, in the following order:
		 *  - for capture
		 *  - for playback
		 */
		return;
	}

	if (!_session) {
		route_latency_value.set_text ("--");
		io_latency_value.set_text ("--");
	} else {
		samplecnt_t wrl = _session->worst_route_latency ();
		samplecnt_t iol = _session->io_latency ();
		float rate      = _session->nominal_sample_rate ();

		route_latency_value.set_text (samples_as_time_string (wrl, rate));

		if (_session->engine().check_for_ambiguous_latency (true)) {
			_ambiguous_latency = true;
			io_latency_value.set_markup ("<span background=\"red\" foreground=\"white\">ambiguous</span>");
		} else {
			_ambiguous_latency = false;
			io_latency_value.set_text (samples_as_time_string (iol, rate));
		}
	}
}

void
TransportBar::parameter_changed (std::string p)
{
	if (p == "external-sync") {
		if (!_session->config.get_external_sync()) {
			sync_button.set_text (S_("SyncSource|Int."));
		} else {
		}
	} else if (p == "sync-source") {
		if (_session) {
			if (!_session->config.get_external_sync()) {
				sync_button.set_text (S_("SyncSource|Int."));
			} else {
				sync_button.set_text (TransportMasterManager::instance().current()->display_name());
			}
		} else {
			/* changing sync source without a session is unlikely/impossible , except during startup */
			sync_button.set_text (TransportMasterManager::instance().current()->display_name());
		}
//		synchronize_sync_source_and_video_pullup ();
//		set_fps_timeout_connection ();

	} else if (p == "punch-out") {
		ActionManager::map_some_state ("Transport", "TogglePunchOut", sigc::mem_fun (_session->config, &SessionConfiguration::get_punch_out));
		if (!_session->config.get_punch_out()) {
//			unset_dual_punch ();
		}
	} else if (p == "punch-in") {
		ActionManager::map_some_state ("Transport", "TogglePunchIn", sigc::mem_fun (_session->config, &SessionConfiguration::get_punch_in));
		if (!_session->config.get_punch_in()) {
//			unset_dual_punch ();
		}
	} else if (p == "show-mini-timeline") {
		repack_transport_hbox ();
	} else if (p == "show-dsp-load-info") {
		repack_transport_hbox ();
	} else if (p == "show-disk-space-info") {
		repack_transport_hbox ();
	} else if (p == "show-toolbar-recpunch") {
		repack_transport_hbox ();
	} else if (p == "show-toolbar-monitoring") {
		repack_transport_hbox ();
	} else if (p == "show-toolbar-selclock") {
		repack_transport_hbox ();
	} else if (p == "show-toolbar-latency") {
		repack_transport_hbox ();
	} else if (p == "show-toolbar-cuectrl") {
		repack_transport_hbox ();
	} else if (p == "show-toolbar-monitor-info") {
		repack_transport_hbox ();
	} else if (p == "show-editor-meter") {
		repack_transport_hbox ();
	} else if (p == "show-secondary-clock") {
		update_clock_visibility ();
	} else if (p == "action-table-columns") {
/*		const uint32_t cols = UIConfiguration::instance().get_action_table_columns ();
		for (int i = 0; i < MAX_LUA_ACTION_BUTTONS; ++i) {
			const int col = i / 2;
			if (cols & (1<<col)) {
				action_script_call_btn[i].show();
			} else {
				action_script_call_btn[i].hide();
			}
		}
		if (cols == 0) {
			scripts_spacer.hide ();
		} else {
			scripts_spacer.show ();
		} */
	} else if (p == "cue-behavior") {
		CueBehavior cb (_session->config.get_cue_behavior());
//		_cue_play_enable.set_active (cb & ARDOUR::FollowCues);
	} else if (p == "record-mode") {
		size_t m = _session->config.get_record_mode ();
		assert (m < record_mode_strings.size ());
		record_mode_selector.set_active (record_mode_strings[m]);
	} else if (p == "no-strobe") {
//		stop_clocking ();
//		start_clocking ();
	}
}


bool
TransportBar::sync_button_clicked (GdkEventButton* ev)
{
	if (ev->button != 3) {
		/* this handler is just for button-3 clicks */
		return false;
	}

	Glib::RefPtr<ToggleAction> tact = ActionManager::get_toggle_action ("Window", "toggle-transport-masters");
	tact->set_active();
	return true;
}

void
TransportBar::set_record_mode (RecordMode m)
{
	if (_session) {
		_session->config.set_record_mode (m);
	}
}

void
TransportBar::sync_blink (bool onoff)
{
	if (_session == 0 || !_session->config.get_external_sync()) {
		/* internal sync */
		sync_button.set_active (false);
		return;
	}

	if (!_session->transport_locked()) {
		/* not locked, so blink on and off according to the onoff argument */

		if (onoff) {
			sync_button.set_active (true);
		} else {
			sync_button.set_active (false);
		}
	} else {
		/* locked */
		sync_button.set_active (true);
	}
}

void
TransportBar::blink_handler (bool blink_on)
{
	sync_blink (blink_on);

	if (UIConfiguration::instance().get_no_strobe() || !UIConfiguration::instance().get_blink_alert_indicators()) {
		blink_on = true;
	}
	solo_blink (blink_on);
	audition_blink (blink_on);
	feedback_blink (blink_on);
}

void
TransportBar::map_transport_state ()
{
	shuttle_box.map_transport_state ();

	if (!_session) {
		record_mode_selector.set_sensitive (false);
		return;
	}

	float sp = _session->transport_speed();

	if (sp != 0.0f) {
		record_mode_selector.set_sensitive (!_session->actively_recording ());
	} else {
		record_mode_selector.set_sensitive (true);
//		update_disk_space ();
	}

}

