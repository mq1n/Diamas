quest discord_lobby begin
	state start begin
		when 9003.chat."Discord Lobby " begin
			say("")
			local s = select("Create lobby ", "Join to lobby ", "Get lobby informations ", "Cancel ")
			if s  == 1 then
				game.create_discord_lobby()
			elseif s == 2 then
				say_title("Enter lobby informations")

				say("Lobby ID")
				local lobby_id = input()

				if lobby_id ~= nil then
					say("Lobby Secret")
					local lobby_secret = input()

					if lobby_secret ~= nil then
						game.join_discord_lobby(lobby_id, lobby_secret)
					end
				end
			elseif s == 3 then
				local lobby_id, lobby_secret = game.get_discord_lobby()
				say_title("Lobby informations")
				say("Lobby ID: " .. lobby_id)
				say("Lobby Secret: " .. lobby_secret)
			elseif s == 4 then
				return
			end
		end
	end
end