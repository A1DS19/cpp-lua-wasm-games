Game = {}
Game.__index = Game

function Game:create(params)
	params = params or {}
	local this = {
		grid = params.grid or Grid:create({ num_rows = 17, offset = vec2(7, 1) }),
		current_tet = get_random_tetromino(),
		next_tet = get_random_tetromino(),
		game_timer = Timer(),
		drop_timer = Timer(),
		drop_time = 2000,
		start_pos = vec2(0, 0),
		next_pos = vec2(0, 0),

		game_over = false,
		game_over_text = nil,

		score_label = nil,
		score_text = nil,
		score_value = 0,

		level_label = nil,
		level_text = nil,
		level = 1,

		lines_cleared = 0,
	}

	local grid_offset = this.grid.offset
	this.start_pos = vec2(grid_offset.x + math.floor(this.grid.num_cols / 2), grid_offset.y + 1)

	this.current_tet:move(this.start_pos.y, this.start_pos.x)

	this.next_pos = vec2(grid_offset.x + this.grid.num_cols + 3, 5)

	this.next_tet:move(this.next_pos.y, this.next_pos.x)

	this.game_over_text = Entity()
	this.game_over_text:add_component(Transform(vec2(400, 300), vec2(1, 1), 0))
	local text = this.game_over_text:add_component(TextComponent("pixel32", "Game Over"))
	text.hidden = true

	this.score_label = Entity()
	local score_transform = this.score_label:add_component(Transform(vec2(16, 32), vec2(1.5, 1.5), 0))
	this.score_label:add_component(TextComponent("pixel16", "Score: "))

	local score_text_w = j2d_measure_text("Score: ", "pixel16")
	this.score_text = Entity()
	this.score_text:add_component(
		Transform(vec2(score_transform.position.x + score_text_w, score_transform.position.y), vec2(1, 1), 0)
	)
	this.score_text:add_component(TextComponent("pixel16", "0"))

	local level_text_w = j2d_measure_text("Level: ", "pixel32")
	this.level_label = Entity()
	local level_transform =
		this.level_label:add_component(Transform(vec2(400 - (level_text_w / 2), 20), vec2(1.5, 1.5), 0))
	this.level_label:add_component(TextComponent("pixel32", "Level: "))

	this.level_text = Entity()
	this.level_text:add_component(
		Transform(vec2(level_transform.position.x + level_text_w, level_transform.position.y), vec2(1, 1), 0)
	)
	this.level_text:add_component(TextComponent("pixel32", "1"))

	this.game_timer:start()
	this.drop_timer:start()

	MusicPlayer.play("main-theme")

	setmetatable(this, self)
	return this
end

function Game:update_tetromino(tet)
	if Keyboard.just_pressed(KEY_S) then
		if tet:can_move(self.grid, 1, 0) then
			tet:move(1, 0)
		end
	elseif Keyboard.just_pressed(KEY_A) then
		if tet:can_move(self.grid, 0, -1) then
			tet:move(0, -1)
		end
	elseif Keyboard.just_pressed(KEY_D) then
		if tet:can_move(self.grid, 0, 1) then
			tet:move(0, 1)
		end
	elseif Keyboard.just_pressed(KEY_E) then
		if tet:can_rotate(self.grid, true) then
			tet:do_rotate_clockwise()
		end
	elseif Keyboard.just_pressed(KEY_Q) then
		if tet:can_rotate(self.grid, false) then
			tet:do_rotate_counter_clockwise()
		end
	end

	local try_lock = false
	if self.drop_timer:elapsed_ms() >= self.drop_time then
		if tet:can_move(self.grid, 1, 0) then
			tet:move(1, 0)
			self.drop_timer:stop()
			self.drop_timer:start()
		else
			try_lock = true
		end
	end

	if try_lock then
		if not self:lock_tetromino() then
			self.game_over = true
		end
	end
end

function Game:update_score(num_rows)
	local points = 1
	if num_rows == 1 then
		points = 100
	elseif num_rows == 2 then
		points = 300
	elseif num_rows == 3 then
		points = 500
	elseif num_rows == 4 then
		points = 800
	end

	points = points * self.level
	self.score_value = self.score_value + points

	local text = self.score_text:get_component(TextComponent)
	text.text = tostring(self.score_value)
end

function Game:update()
  if self.game_over then
    self:on_game_over()
    return
  end

  self:update_tetromino(self.current_tet)
end

function Game:check_level_up()
  -- handle level up
end

function Game:on_game_over()
  -- handle game over
end

function Game:lock_tetromino()
  if not self.current_tet:lock_to_grid(self.grid) then
    SoundPlayer.play("death")
    MusicPlayer.stop()
    MusicPlayer.play("game-over", 1)
    self.game_over_text:get_component(TextComponent).hidden = false
    return false
  end

  self.current_tet = nil
  self.current_tet = self.next_tet
  self.next_tet = nil
  self.next_tet = get_random_tetromino()
  self.next_tet:move(self.next_pos.y, self.next_pos.x)

  self.current_tet:reset()
  self.current_tet:move(self.start_pos.y, self.start_pos.x)

  SoundPlayer.play("bump")

  local num_rows_cleared = self.grid:clear_full_rows()
  if num_rows_cleared > 0 then
    SoundPlayer.play("finish-row")
    self:update_score(num_rows_cleared)
    self.lines_cleared = self.lines_cleared + num_rows_cleared
  end

  self:check_level_up()
  self.drop_timer:restart()
  return true
end
