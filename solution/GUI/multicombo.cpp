#include "multicombo.h"
#include "child.h"
#include "../Math.h"

void c_multicombo::update_format()
{
	format.clear();
	int t = 0;
	for (int i = 0; i < elements.size(); i++) {
		if (*(unsigned int*)value & 1 << i) {
			if (t > 0) format += ", ";
			format += elements[i];
			t++;
		}
	}
	if (!format.size())
		format = "...";
	ImGui::PushFont(fonts::menu_desc);
	format_size = ImGui::CalcTextSize(format.c_str()).x;
	ImGui::PopFont();
}

bool c_multicombo::hovered()
{
	if (should_render)
		if (!should_render())
			return false;
	auto pos = ((c_child*)child)->get_cursor_position();
	if (!((c_child*)child)->hovered())
		return false;
	auto size = Vector2D(g_size, 20);
	return g_mouse.x >= pos.x && g_mouse.y >= pos.y
		&& g_mouse.x <= pos.x + size.x && g_mouse.y <= pos.y + size.y;
}
bool c_multicombo::update()
{
	if (should_render)
		if (!should_render())
			return false;
	c_child* c = (c_child*)child;
	if (!c) return false;
	
	auto wnd = (c_window*)c->get_parent();
	if (!wnd) return false;
	if (wnd->get_active_tab_index() != this->tab && wnd->get_tabs().size() > 0)
		return false;
	if (wnd->g_active_element != this && wnd->g_active_element != nullptr) return false;
	auto pos = c->get_cursor_position();
	auto size = Vector2D(g_size, element_height);

	bool h = hovered();
	if (h) wnd->g_hovered_element = this;
	
	if (h && c->hovered() && wnd->is_click()) {
		if (open) {
			open = false;
			wnd->g_active_element = nullptr;
		}
		else
		{
			open = true;
			wnd->g_active_element = this;
		}
		return true;
	}
	update_format();
	if (open) {
		if (open_animation < 1.f)
			open_animation += animation_speed;

		h = g_mouse.x >= pos.x && g_mouse.y >= pos.y
			&& g_mouse.x <= pos.x + size.x && g_mouse.y <= pos.y + size.y * (elements.size() + 1);
		if (wnd->is_click() && !h) {
			open = false;
			wnd->g_active_element = nullptr;
			wnd->reset_mouse();
			return true;
		}
		if (open_animation == 1.f) {
			pos.y += 5;
			for (size_t i = 0; i < elements.size(); i++) {
				pos.y += size.y;
				h = g_mouse.x >= pos.x && g_mouse.y >= pos.y
					&& g_mouse.x <= pos.x + size.x && g_mouse.y < pos.y + size.y;
				if (h) {
					if (animations[i] < 1.f) animations[i] += animation_speed;
					if (h) wnd->g_hovered_element = this;
				}
				else
				{
					if (animations[i] > 0.f) animations[i] -= animation_speed;
				}
				if (animations[i] > 1.f) animations[i] = 1.f;
				else if (animations[i] < 0.f) animations[i] = 0.f;

				if (wnd->is_click() && h) {
					*(unsigned int*)value ^= 1 << i;
					return false;
				}
			}
		}
		wnd->g_active_element = this;
		for (int i = 0; i < elements.size(); i++) {
			if (i == 0 || animations2[i - 1] >= 0.5f)
				animations2[i] = clamp(animations2[i] + animation_speed, 0.f, 1.f);
		}
	}
	else
	{
		for (int i = 0; i < elements.size(); i++)
			this->animations2[i] = 0.f;

		hovering_animation = 0.f;
		
		if (wnd->g_active_element == this)
			wnd->g_active_element = nullptr;
		if (open_animation > 0.f)
			open_animation -= animation_speed;
	}
	// bullshit animations
	if (h || open) {
		if (animation < 1.f) animation += animation_speed;
	}
	else {
		if (animation > 0.f) animation -= animation_speed;
	}
	animation = clamp(animation, 0.f, 1.f);
	
	if (open_animation > 1.f) open_animation = 1.f;
	else if (open_animation < 0.f) open_animation = 0.f;
	return false;
}
void c_multicombo::render() {
	if (should_render)
		if (!should_render())
			return;
	c_child* c = (c_child*)child;
	if (!c)
		return;
	auto wnd = (c_window*)c->get_parent();
	if (!wnd)
		return;
	if (wnd->get_active_tab_index() != tab && wnd->get_tabs().size() > 0)
		return;
	auto pos = c->get_cursor_position();
	auto alpha = (int)(wnd->get_transparency() * 2.55f) * (1.f - showing_animation);
	auto size = Vector2D(g_size, element_height);

	bool h = hovered();

	auto clr = menu_colors::background2.manage_alpha( alpha - 15.f * animation);

	if (label.size() > 0) {
		g_Render->DrawString(pos.x, pos.y, color_t(200 + 55.f * animation, 200 + 55.f * animation, 200 + 55.f * animation, alpha),
			render::none, fonts::menu_desc, label.c_str());
	}

	ImGui::PushFont(fonts::menu_desc);
	float label_size = ImGui::CalcTextSize("Select").x;
	ImGui::PopFont();

	int arrow_size = 20;
	int arrow_offset = 3;
	g_Render->DrawLine(
		pos.x + size.x - arrow_size / 2,
		pos.y + arrow_size / 4 + arrow_offset,
		pos.x + size.x - arrow_size / 4 * sin(DEG2RAD(animation * 90.f)),
		pos.y + arrow_size / 2 * sin(DEG2RAD(animation * 90.f)) + arrow_offset,
		color_t(255, 255, 255, (int)(alpha * animation)), 2.f);

	g_Render->DrawLine(
		pos.x + size.x,
		pos.y + arrow_size / 4 + arrow_offset,
		pos.x + size.x - arrow_size / 4 * sin(DEG2RAD(animation * 90.f)),
		pos.y + arrow_size / 2 * sin(DEG2RAD(animation * 90.f)) + arrow_offset,
		color_t(255, 255, 255, (int)(alpha * animation)), 2.f);


	g_Render->DrawString(pos.x + size.x - label_size - arrow_size * animation * 0.75f, pos.y, color_t(200 + 55.f * animation, 200 + 55.f * animation, 200 + 55.f * animation, alpha),
		render::none, fonts::menu_desc, "Select");

	if (open) {
		auto new_alpha = clamp(alpha * open_animation, 0.f, 255.f);
		g_Render->FilledRect(pos.x - 4, pos.y + size.y, size.x + 8, size.y * elements.size(), color_t(40, 49, 70, new_alpha), 10.f);

		for (size_t i = 0; i < elements.size(); i++) {

			ImGui::PushFont(fonts::menu_desc);
			float element_size = ImGui::CalcTextSize(elements[i].c_str()).x;
			ImGui::PopFont();
			pos.y += size.y;

			/*g_Render->DrawString(pos.x + size.x - element_size - 15.f * (1.f - animations2[i]), pos.y + size.y / 2, *(int*)value == i
				? menu_colors::main_color.manage_alpha((new_alpha - 50.f * (1.f - animations[i])) * animations2[i])
				: color_t(200 + animations[i] * 55.f, 200 + animations[i] * 55.f, 200 + animations[i] * 55.f, new_alpha * animations2[i]), render::centered_y,
				fonts::menu_desc, elements[i].c_str());*/
			g_Render->DrawString(pos.x + size.x - element_size - 15.f * (1.f - animations2[i]), pos.y + size.y / 2,
				((*(unsigned int*)value) & 1 << i) ?
				menu_colors::main_color.manage_alpha((new_alpha - 50.f * (1.f - animations[i])) * animations2[i])
				: color_t(200 + animations[i] * 55.f, 200 + animations[i] * 55.f, 200 + animations[i] * 55.f, new_alpha * animations2[i]),
				render::centered_y, fonts::menu_desc, elements[i].c_str());
		}
	}
}

int c_multicombo::get_total_offset() {
	if (should_render)
		if (!should_render())
			return 0;
	return 20 * (!label.empty());
};