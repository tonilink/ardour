ardour {
	["type"] = "EditorAction",
	name = "Create MIDI Drum Tracks V2",
	author = "Toni Link",
	description = [[Creates 12 new tracks with representative drum names and color. Based on the original script by PSmith.]]
}


function factory () return function ()
		local names = {
			"Kick",
			"Snare",
			"Rim",
			"Clap",
			"CHh",
			"OHh",
			"Cymb",
			"Crash",
			"LoTo",
			"HiTo",
			"FlTo",
			"Shaker"
		}

		local color = 0xff8800ff  --orange

		local i = 1
		while names[i] do
		  local tl = Session:new_midi_track(
			ARDOUR.ChanCount(ARDOUR.DataType ("midi"), 1),
								 ARDOUR.ChanCount(ARDOUR.DataType ("midi"), 1),
								 true,
						   ARDOUR.PluginInfo(), nil,
								 ARDOUR.RouteGroup(),
								 1, names[i],
						   ARDOUR.PresentationInfo.max_order, ARDOUR.TrackMode.Normal, true)

			for track in tl:iter () do
				track:presentation_info_ptr ():set_color (color)
			end

			i = i + 1
		end --foreach track

end end -- function factory

function icon (params)
return function (ctx, width, height)
ctx:set_source_rgb (0.25, 0.25, 0.25)
ctx:rectangle (width * 0.125, height * 0.125, width * 0.75, height * 0.75)
ctx:fill ()


ctx:set_source_rgb (0.05, 0.05, 0.05)
ctx:rectangle (width * 0.1875, height * 0.1875, width * 0.625, height * 0.125)
ctx:fill ()


ctx:set_source_rgb (0.8, 0.8, 0.8)


local pad_w = width * 0.125
local pad_h = height * 0.125


ctx:rectangle (width * 0.1875, height * 0.4375, pad_w, pad_h)
ctx:rectangle (width * 0.4375, height * 0.4375, pad_w, pad_h)
ctx:rectangle (width * 0.6875, height * 0.4375, pad_w, pad_h)


ctx:rectangle (width * 0.1875, height * 0.6875, pad_w, pad_h)
ctx:rectangle (width * 0.4375, height * 0.6875, pad_w, pad_h)
ctx:rectangle (width * 0.6875, height * 0.6875, pad_w, pad_h)


ctx:fill ()
end
end
