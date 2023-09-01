#include "memory.hpp"

#include <fmt/format.h>
#include <imgui.h>

#include <charconv>
#include <numeric>

namespace
{

const ImVec2    MARGIN{10, 10};
const ImVec2    ALLOCATION_GAP{0, 32};
constexpr float LINE_THICKNESS = 3.0F;
constexpr ImU32 INITIALISED    = IM_COL32(0, 150, 255, 255);
constexpr ImU32 UNINITIALISED  = IM_COL32(255, 50, 50, 255);

ImVec2 operator+(ImVec2 lhs, ImVec2 rhs) {
	return {lhs.x + rhs.x, lhs.y + rhs.y};
}

ImVec2 operator-(ImVec2 lhs, ImVec2 rhs) {
	return {lhs.x - rhs.x, lhs.y - rhs.y};
}

ImVec2 operator*(ImVec2 lhs, ImVec2 rhs) {
	return {lhs.x * rhs.x, lhs.y * rhs.y};
}

ImVec2 operator*(float lhs, ImVec2 rhs) {
	return {lhs * rhs.x, lhs * rhs.y};
}

ImVec2 operator/(ImVec2 lhs, float rhs) {
	return {lhs.x / rhs, lhs.y / rhs};
}

void operator+=(ImVec2 &lhs, ImVec2 rhs) {
	lhs = lhs + rhs;
}

auto address_from_string(char const *value) -> uintptr_t {
	uintptr_t              address = 0;
	std::string            string  = value;
	std::from_chars_result result  = std::from_chars(
            // Skip the 0x in hexadecimal
            string.data() + 2,
            string.data() + string.length(),
            address,
            16); // hexadecimal base
	return result.ec == std::errc() ? address : 0;
}

} // namespace

namespace visual::components
{

VisualAddressBoudingBox::VisualAddressBoudingBox(ImVec2 centre, ImVec2 size)
    : centre(centre)
    , size(size) {
}

auto VisualAddressBoudingBox::intersect(ImVec2 from) -> ImVec2 {
	ImVec2 direction           = from - centre;
	bool   closer_to_vertical  = std::abs(direction.y) > std::abs(direction.x);
	float  perpendicular_scale = std::abs(closer_to_vertical ? direction.y : direction.x);
	float  bounds_scale        = std::abs(closer_to_vertical ? size.y : size.x);
	return centre + bounds_scale * direction / perpendicular_scale;
}

auto Allocation_Element::leaf() const -> bool {
	return m_fields.empty();
}

auto Allocation_Element::initialised() const -> bool {
	return m_state == State::Initialised;
}

void Allocation_Element::move() {
	m_state = State::Moved;
}

void Allocation_Element::assign(std::string value) {
	m_state = State::Initialised;
	update_value(std::move(value));
}

void Allocation_Element::construct(std::string value) {
	m_state = State::Initialised;
	update_value(std::move(value));
}

void Allocation_Element::destroy() {
	m_state = State::Uninitialised;
}

void Allocation_Element::update_value(std::string &&value) {
	m_value = std::move(value);
}

auto Allocation_Element::contains_address(uintptr_t target) const -> bool {
	return target >= address() && target < address() + element_size();
}

void Allocation_Element::draw_values(ImDrawList *draw, ImVec2 position, VisualAddressElementMap &address_map) {
	if (!leaf())
	{
		for (auto const &field : m_fields)
		{
			field->draw_values(draw, position, address_map);
			position += ImVec2(0, field->size().y);
		}
		return;
	}

	address_map[address()] = VisualAddressBoudingBox(position + size() / 2.0f, size() / 2.0f);
	draw->AddRectFilled(
	    position,
	    position + size(),
	    initialised() ? INITIALISED : UNINITIALISED,
	    16,
	    ImDrawCornerFlags_All);

	if (!pointer())
	{
		ImGui::SetCursorScreenPos(MARGIN + position);
		ImGui::Text("%s", m_value.c_str());
	}
}

void Allocation_Element::draw_pointers(ImDrawList *draw, VisualAddressElementMap const &address_map) {
	if (!leaf())
	{
		for (auto const &field : m_fields)
		{
			field->draw_pointers(draw, address_map);
		}
		return;
	}

	if (!pointer())
	{
		return;
	}

	ImVec2                  source = address_map.at(address()).centre;
	VisualAddressBoudingBox target = address_map.at(address_from_string(m_value.c_str()));
	draw->AddLine(source, target.intersect(source), 0x77FF'FFFFU, LINE_THICKNESS);
}

auto Allocation_Element::size() const -> ImVec2 {
	ImVec2 size;
	if (!leaf())
	{
		return std::accumulate(
		    m_fields.begin(),
		    m_fields.end(),
		    ImVec2(),
		    [](ImVec2 size, Element const &element) {
			    return ImVec2(
				std::max(size.x, element->size().x),
				size.y + element->size().y);
		    });
	}

	if (pointer())
	{
		return 3 * MARGIN;
	}

	return 2 * MARGIN + ImGui::CalcTextSize(m_value.c_str());
}

Allocation_Block::Allocation_Block(Allocation_Type allocation_type)
    : m_allocation_type(allocation_type) {
}

auto Allocation_Block::count() const -> size_t {
	return m_elements.size();
}

auto Allocation_Block::heap() const -> bool {
	return m_allocation_type == Allocation_Type::Heap;
}

auto Allocation_Block::stack() const -> bool {
	return m_allocation_type == Allocation_Type::Stack;
}

auto Allocation_Block::match_address(uintptr_t target) const -> bool {
	return address() == target;
}

auto Allocation_Block::contains(uintptr_t target) const -> bool {
	return target >= address() && target < address() + element_size() * count();
}

auto Allocation_Block::all_elements_destroyed() const -> bool {
	return std::none_of(m_elements.begin(), m_elements.end(), [](Element const &element) {
		return element->initialised();
	});
}

auto Allocation_Block::element_index(uintptr_t element_address) const -> size_t {
	assert(contains(element_address) && "Expected the address to be inside of the allocation");

	auto base_address     = address();
	auto relative_address = element_address - base_address;
	return relative_address / element_size();
}

auto Allocation_Block::element(uintptr_t element_address) -> Allocation_Element & {
	return *m_elements[element_index(element_address)];
}

auto Allocation_Block::element(uintptr_t element_address) const -> Allocation_Element const & {
	return *m_elements[element_index(element_address)];
}

void Allocation_Block::draw_values(ImDrawList *draw, ImVec2 position, VisualAddressElementMap &address_map) {
	address_map[address()] = VisualAddressBoudingBox(position + size() / 2.0f, size() / 2.0f);
	draw->AddRectFilled(position, position + size(), ImColor(64, 64, 64, 200), 16, ImDrawCornerFlags_All);

	for (auto &element : m_elements)
	{
		element->draw_values(draw, MARGIN + position, address_map);
		position += ImVec2(1, 0) * (MARGIN + element->size());
	}
}

void Allocation_Block::draw_pointers(ImDrawList *draw, VisualAddressElementMap const &address_map) {
	for (auto &element : m_elements)
	{
		element->draw_pointers(draw, address_map);
	}
}

auto Allocation_Block::size() const -> ImVec2 {
	return MARGIN
	       + std::accumulate(
		   m_elements.begin(),
		   m_elements.end(),
		   ImVec2(),
		   [](ImVec2 size, Element const &element) {
			   ImVec2 element_size = MARGIN + element->size();
			   return ImVec2(size.x + element_size.x, std::max(size.y, element_size.y));
		   });
}

void Memory::draw() {
	bool             open  = true;
	ImGuiWindowFlags flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove
				 | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings;

	if (ImGui::Begin("Memory Representation", &open, flags))
	{
		ImDrawList *draw        = ImGui::GetWindowDrawList();
		auto        address_map = draw_allocations(draw);
		draw_pointers(draw, address_map);
	}
	ImGui::End();
}

void Memory::update() {
	Update_Has_Visual_Effect has_visual_effect = false;
	while (!m_events.empty() && !has_visual_effect)
	{
		auto event = m_events.front();
		m_events.pop_front();
		has_visual_effect = event();
	}
}

auto Memory::draw_allocations(ImDrawList *draw) -> VisualAddressElementMap {
	ImVec2                  position;
	VisualAddressElementMap address_map;

	for (auto &allocation : m_allocations)
	{
		if (allocation->stack())
		{
			continue;
		}

		allocation->draw_values(draw, MARGIN + position, address_map);
		position += ALLOCATION_GAP + ImVec2(0, allocation->size().y);
	}

	draw_nullptr(draw, position, address_map);

	return address_map;
}

void Memory::draw_nullptr(ImDrawList *draw, ImVec2 position, VisualAddressElementMap &address_map) {
	char const *value = "nullptr";
	ImVec2      size  = 2 * MARGIN + ImGui::CalcTextSize(value);
	address_map[0]    = VisualAddressBoudingBox(position + size / 2.0f, size / 2.0f);
	draw->AddRectFilled(position, position + size, INITIALISED, 16, ImDrawCornerFlags_All);

	ImGui::SetCursorScreenPos(MARGIN + position);
	ImGui::Text("%s", value);
}

void Memory::draw_pointers(ImDrawList *draw, VisualAddressElementMap const &address_map) {
	for (auto &allocation : m_allocations)
	{
		if (allocation->stack())
		{
			continue;
		}

		allocation->draw_pointers(draw, address_map);
	}
}

std::optional<std::reference_wrapper<Allocation_Block>> Memory::allocation_containing(
    uintptr_t raw_address) {
	for (auto &allocation : m_allocations)
	{
		if (allocation->contains(raw_address))
		{
			return *allocation;
		}
	}
	return std::nullopt;
}

} // namespace visual::components
