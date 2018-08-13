#pragma once

namespace renderer {

	void init(graphic::cRenderer &renderer);

	void update(graphic::cRenderer &renderer);

	void render_frame(graphic::cRenderer &renderer);

	void present_frame();

	int resize(LPARAM lparam);


	Microsoft::WRL::ComPtr<ID3D11SamplerState> &get_sampler();

	Microsoft::WRL::ComPtr<ID3D11RasterizerState> &get_rasterizer_state();

	Microsoft::WRL::ComPtr<ID3D11DepthStencilState> &get_depth_stencil_state();

	Microsoft::WRL::ComPtr<ID3D11BlendState> &get_blend_state_01();

	Microsoft::WRL::ComPtr<ID3D11BlendState> &get_blend_state_0011();
	
	void create_cb(graphic::cRenderer &renderer
		, Microsoft::WRL::ComPtr<ID3D11Buffer>& com_buffer, const void *p_data, uint32_t size);

	void update_cb(graphic::cRenderer &renderer
		, Microsoft::WRL::ComPtr<ID3D11Buffer>& com_buffer, const void *p_data, uint32_t data_size);

	void compile_and_create_shader(graphic::cRenderer &renderer
		, Microsoft::WRL::ComPtr<ID3D11VertexShader>& com_vs, const wchar_t* p_name);

	void compile_and_create_shader(graphic::cRenderer &renderer
		, Microsoft::WRL::ComPtr<ID3D11PixelShader>& com_ps, const wchar_t* p_name, const D3D_SHADER_MACRO *p_macros = NULL);

	void compile_and_create_shader(graphic::cRenderer &renderer
		, Microsoft::WRL::ComPtr<ID3D11GeometryShader>& com_gs, const wchar_t* p_name);

	bool check_full_precision_rgb_support();

} // namespace renderer