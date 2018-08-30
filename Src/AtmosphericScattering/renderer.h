#pragma once

namespace renderer {

	void init(graphic::cRenderer &renderer);

	void update(graphic::cRenderer &renderer);

	void render_frame(graphic::cRenderer &renderer);

	void present_frame();

	Vector3 GetCameraPos();

	Microsoft::WRL::ComPtr<ID3D11SamplerState> &get_sampler();

	Microsoft::WRL::ComPtr<ID3D11DepthStencilState> &get_depth_stencil_state();

	Microsoft::WRL::ComPtr<ID3D11BlendState> &get_blend_state_01();

	Microsoft::WRL::ComPtr<ID3D11BlendState> &get_blend_state_0011();
	
	bool check_full_precision_rgb_support();

} // namespace renderer