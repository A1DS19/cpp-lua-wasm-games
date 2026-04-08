Grid = {}
Grid.__index = Grid

function Grid:create(params)
	params = params or {}
	local this = {
		cell_size = params.cell_size or 32,
		num_rows = params.num_rows or 17,
		num_cols = params.num_cols or 10,
		offset = params.offset or vec2(7, 0),
		position = vec2(0, 0),
		grid = {},
		gridcells = {},
	}

	this.position.x = this.offset.x * this.cell_size
	this.position.y = this.offset.y * this.cell_size

	local ROWS = this.num_rows + 1
	local COLS = this.num_cols + 1

	for row = 0, ROWS do
		for col = 0, COLS do
			local entity = Entity()
			entity:add_component(
				Transform(
					vec2(this.position.x + (col * this.cell_size), this.position.y + (row * this.cell_size)),
					vec2(1, 1),
					0.0
				)
			)

			local color = J2D_WHITE
			local start_x = 0

			if row - 1 < 0 or row > this.num_rows or col - 1 < 0 or col > this.num_cols then
				color = Color(135, 135, 135, 255)
				start_x = 1
			end

			local sprite = entity:add_component(Sprite("blocks", 32, 32, 0, start_x, 0, color))
			sprite:generate_uvs(64, 32)

			this.grid[row * COLS + col] = entity
		end
	end

	for i = 0, this.num_rows * this.num_cols do
		this.gridcells[i] = nil
	end

	setmetatable(this, self)
	return this
end

function Grid:inside_grid(row, col)
	if row > 0 and row <= self.num_rows and col > 0 and col <= self.num_cols then
		return true
	end

	return false
end

function Grid:inside_cols(col)
	if col < 0 then
		return 1
	elseif col > self.num_cols then
		return -1
	end

	return 0
end

function Grid:is_cell_empty(row, col)
	if self.gridcells[row * self.num_cols + col] == nil then
		return true
	end

	return false
end

function Grid:is_row_full(row)
	for col = 1, self.num_cols do
		if self.gridcells[row * self.num_cols + col] == nil then
			return false
		end
	end

	return true
end

function Grid:clear_row(row)
	for col = 1, self.num_cols do
		local index = row * self.num_cols + col
		local block = self.gridcells[index]
		block:destroy()
		self.gridcells[index] = nil
	end
end

function Grid:move_row_down(row, numrows)
	for col = 1, self.num_cols do
		local curr_cell = row * self.num_cols + col
		local cell_below = (row + numrows) * self.num_cols + col
		self.gridcells[cell_below] = self.gridcells[curr_cell]
		self.gridcells[curr_cell] = nil

		local block = self.gridcells[cell_below]
		if block then
			block:dropdown(row + numrows + self.offset.y)
		end
	end
end

function Grid:set_cell(row, col, block)
	if self:is_cell_empty(row, col) then
		self.gridcells[row * self.num_cols + col] = block
		return true
	end

	return false
end

function Grid:clear_full_rows()
	local rows_cleared = 0
	for row = self.num_rows, 1, -1 do
		if self:is_row_full(row) then
			self:clear_row(row)
			rows_cleared = rows_cleared + 1
		elseif rows_cleared > 0 then
			self:move_row_down(row, rows_cleared)
		end
	end

	return rows_cleared
end
