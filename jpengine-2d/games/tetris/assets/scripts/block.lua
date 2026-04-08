Block = {}
Block.__index = Block

function Block:Create(params)
	local this = {
		cell_size = params.cell_size or 32,
		row = params.row or 0,
		col = params.col or 0,
		id = -1,
		entity = nil,
	}

	setmetatable(this, self)
	return this
end

function Block:set_position(row_offset, col_offset)
	if self.entity == nil then
		print("trying to set position of uninitialized block")
	end

	local transform = self.entity.get_component(Transform)
	transform.position = vec2((self.col + col_offset) * self.cell_size, (self.row + col_offset) * self.cell_size)
end

function Block:dropdown(row_offset)
	if self.entity == nil then
		print("trying to dropdown of uninitialized block")
	end

	local transform = self.entity.get_component(Transform)
	transform.position.y = row_offset * self.cell_size
end

function Block:destroy()
	if self.entity ~= nil then
		self.entity:destroy()
	end
end

Position = {}
Position.__index = Position

function Position:create(r, c)
	local this = {
		row = r,
		col = c,
	}
	setmetatable(this, self)
	return this
end

ETetrominoType = {
	LType = 1,
	JType = 2,
	IType = 3,
	OType = 4,
	SType = 5,
	TType = 6,
	ZType = 7,
}
