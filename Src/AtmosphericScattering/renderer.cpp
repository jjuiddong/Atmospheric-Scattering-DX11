
#include "stdafx.h"
#include "renderer.h"
#include <dxgi1_5.h>
#include "gui.h"
#include "atmosphere.h"

using namespace graphic;


namespace renderer {

	ComPtr<ID3D11DepthStencilState> com_depth_stencil_state = nullptr;
	ComPtr<ID3D11BlendState> com_blend_state_01 = nullptr;
	ComPtr<ID3D11BlendState> com_blend_state_0011 = nullptr;

	static bool is_full_precision_rgb_supported = false;
	static int last_predefined_view_index = 0;
	static float fov_y_angle_deg = 50.f;
	static float near_plane = 1.0;
	
	struct AtmosphereConstantBuffer 
	{
		XMFLOAT4X4 view_from_clip;
		XMFLOAT4X4 world_from_view;
		XMFLOAT3 view_pos_ws;
		float sun_disk_size_x;
		XMFLOAT3 earth_center_pos_ws;
		float sun_disk_size_y;
		XMFLOAT3 sun_direction_ws;
		float    exposure;
		XMFLOAT3 white_point;
		int layer;
		XMFLOAT4X3 luminance_from_radiance;
		int scattering_order;
		float _pad[3];
	};

	cConstantBuffer<AtmosphereConstantBuffer> cbAtmosphere;


	bool check_full_precision_rgb_support() {
		return is_full_precision_rgb_supported;
	}


	void init(graphic::cRenderer &renderer) 
	{

		UINT format_support_flags = 0;
		DXGI_FORMAT full_precision_format = DXGI_FORMAT_R32G32B32_FLOAT;
		renderer.GetDevice()->CheckFormatSupport(full_precision_format, &format_support_flags);
		is_full_precision_rgb_supported = ((format_support_flags & D3D11_FORMAT_SUPPORT_RENDER_TARGET) != 0);


		{
			cbAtmosphere.Create(renderer);

			{
				D3D11_DEPTH_STENCIL_DESC depth_stencil_desc = {};
				HRESULT h_result = renderer.GetDevice()->CreateDepthStencilState(&depth_stencil_desc, com_depth_stencil_state.GetAddressOf());
				//if(h_result != S_OK) error_win32("CreateRasterizerState", (DWORD)h_result);
			}
			
			{
				{
					D3D11_BLEND_DESC blend_state_desc = {};
					blend_state_desc.IndependentBlendEnable = true;
					blend_state_desc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
					blend_state_desc.RenderTarget[1].BlendEnable = true;
					blend_state_desc.RenderTarget[1].SrcBlend = D3D11_BLEND_ONE;
					blend_state_desc.RenderTarget[1].DestBlend = D3D11_BLEND_ONE;
					blend_state_desc.RenderTarget[1].BlendOp = D3D11_BLEND_OP_ADD;
					blend_state_desc.RenderTarget[1].SrcBlendAlpha = D3D11_BLEND_ONE;
					blend_state_desc.RenderTarget[1].DestBlendAlpha = D3D11_BLEND_ONE;
					blend_state_desc.RenderTarget[1].BlendOpAlpha = D3D11_BLEND_OP_ADD;
					blend_state_desc.RenderTarget[1].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
					HRESULT h_result = renderer.GetDevice()->CreateBlendState(&blend_state_desc, com_blend_state_01.GetAddressOf());
				}
				{
					D3D11_BLEND_DESC blend_state_desc = {};
					blend_state_desc.IndependentBlendEnable = true;
					blend_state_desc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
					blend_state_desc.RenderTarget[1].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
					blend_state_desc.RenderTarget[2].BlendEnable = true;
					blend_state_desc.RenderTarget[2].SrcBlend = D3D11_BLEND_ONE;
					blend_state_desc.RenderTarget[2].DestBlend = D3D11_BLEND_ONE;
					blend_state_desc.RenderTarget[2].BlendOp = D3D11_BLEND_OP_ADD;
					blend_state_desc.RenderTarget[2].SrcBlendAlpha = D3D11_BLEND_ONE;
					blend_state_desc.RenderTarget[2].DestBlendAlpha = D3D11_BLEND_ONE;
					blend_state_desc.RenderTarget[2].BlendOpAlpha = D3D11_BLEND_OP_ADD;
					blend_state_desc.RenderTarget[2].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
					blend_state_desc.RenderTarget[3].BlendEnable = true;
					blend_state_desc.RenderTarget[3].SrcBlend = D3D11_BLEND_ONE;
					blend_state_desc.RenderTarget[3].DestBlend = D3D11_BLEND_ONE;
					blend_state_desc.RenderTarget[3].BlendOp = D3D11_BLEND_OP_ADD;
					blend_state_desc.RenderTarget[3].SrcBlendAlpha = D3D11_BLEND_ONE;
					blend_state_desc.RenderTarget[3].DestBlendAlpha = D3D11_BLEND_ONE;
					blend_state_desc.RenderTarget[3].BlendOpAlpha = D3D11_BLEND_OP_ADD;
					blend_state_desc.RenderTarget[3].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
					HRESULT h_result = renderer.GetDevice()->CreateBlendState(&blend_state_desc, com_blend_state_0011.GetAddressOf());
				}
			}
		}
	}

	void set_view(gui::GuiData& gui_data, float view_distance, float view_zenith_angle_in_radians, float view_azimuth_angle_in_radians, float sun_zenith_angle_in_radians, float sun_azimuth_angle_in_radians,float exposure) {
		gui_data.view_distance = view_distance;
		gui_data.view_zenith_angle_in_degrees = XMConvertToDegrees(view_zenith_angle_in_radians);
		gui_data.view_azimuth_angle_in_degrees = XMConvertToDegrees(view_azimuth_angle_in_radians);
		gui_data.sun_zenith_angle_in_degrees = XMConvertToDegrees(sun_zenith_angle_in_radians);
		gui_data.sun_azimuth_angle_in_degrees = XMConvertToDegrees(sun_azimuth_angle_in_radians);
		gui_data.exposure = exposure;
	}

	void update(cRenderer &renderer) {
		gui::GuiData& gui_data = gui::get_data();
		if(last_predefined_view_index != gui_data.predefined_view_index) {
			float exposure_luminance_factor = gui_data.use_luminance != atmosphere::Luminance::NONE ? 1e-5f : 1.0f;
			switch(gui_data.predefined_view_index) {
				case 1: { set_view(gui_data, 9000.0f, 1.47f, 0.0f, 1.3f, 3.0f, 10.0f * exposure_luminance_factor); } break;
				case 2: { set_view(gui_data, 9000.0f, 1.47f, 0.0f, 1.564f, -3.0f, 10.0f * exposure_luminance_factor); } break;
				case 3: { set_view(gui_data, 7000.0f, 1.57f, 0.0f, 1.54f, -2.96f, 10.0f * exposure_luminance_factor); } break;
				case 4: { set_view(gui_data, 7000.0f, 1.57f, 0.0f, 1.328f, -3.044f, 10.0f * exposure_luminance_factor); } break;
				case 5: { set_view(gui_data, 9000.0f, 1.39f, 0.0f, 1.2f, 0.7f, 10.0f * exposure_luminance_factor); } break;
				case 6: { set_view(gui_data, 9000.0f, 1.5f, 0.0f, 1.628f, 1.05f, 200.0f * exposure_luminance_factor); } break;
				case 7: { set_view(gui_data, 7000.0f, 1.43f, 0.0f, 1.57f, 1.34f, 40.0f * exposure_luminance_factor); } break;
				case 8: { set_view(gui_data, 2.7e6f, 0.81f, 0.0f, 1.57f, 2.0f, 10.0f * exposure_luminance_factor); } break;
				case 9: { set_view(gui_data, 1.2e7f, 0.0f, 0.0f, 0.93f, -2.0f, 10.0f * exposure_luminance_factor); } break;
				default: break;
			}
			last_predefined_view_index = gui_data.predefined_view_index;
		}

		{
			float fov_y_angle_rad = XMConvertToRadians(fov_y_angle_deg);
			float aspect_ratio = renderer.m_viewPort.m_vp.Width / renderer.m_viewPort.m_vp.Height;
			float scale_y = (float)(1.0 / tan(fov_y_angle_rad / 2.0));
			float scale_x = scale_y / aspect_ratio;
			XMFLOAT4X4 clip_from_view = { // left-handed reversed-z infinite projection
				scale_x, 0.0, 0.0, 0.0,
				0.0, scale_y, 0.0, 0.0,
				0.0, 0.0, 0.0, near_plane,
				0.0, 0.0, 1.0, 0.0
			};

			XMMATRIX view_from_clip = XMMatrixInverse(nullptr, XMLoadFloat4x4(&clip_from_view));
			XMStoreFloat4x4(&cbAtmosphere.m_v->view_from_clip, view_from_clip);

			float cos_theta = (float)cos(XMConvertToRadians(gui_data.view_zenith_angle_in_degrees));
			float sin_theta = (float)sin(XMConvertToRadians(gui_data.view_zenith_angle_in_degrees));
			float cos_phi = (float)cos(XMConvertToRadians(gui_data.view_azimuth_angle_in_degrees));
			float sin_phi = (float)sin(XMConvertToRadians(gui_data.view_azimuth_angle_in_degrees));

			XMFLOAT3 view_x_ws = { -sin_phi, cos_phi, 0.f };
			XMFLOAT3 view_y_ws = { -cos_theta * cos_phi, -cos_theta * sin_phi, sin_theta };
			XMFLOAT3 view_z_ws = { -sin_theta * cos_phi, -sin_theta * sin_phi, -cos_theta };
			XMFLOAT4X4 world_from_view = {
				view_x_ws.x, view_y_ws.x, view_z_ws.x, -view_z_ws.x * gui_data.view_distance / 1000.f,
				view_x_ws.y, view_y_ws.y, view_z_ws.y, -view_z_ws.y * gui_data.view_distance / 1000.f,
				view_x_ws.z, view_y_ws.z, view_z_ws.z, -view_z_ws.z * gui_data.view_distance / 1000.f,
				0.0, 0.0, 0.0, 1.0
			};
			cbAtmosphere.m_v->world_from_view = world_from_view;

			cbAtmosphere.m_v->view_pos_ws = { world_from_view(0,3),world_from_view(1,3),world_from_view(2,3) };
			cbAtmosphere.m_v->earth_center_pos_ws = { 0.f,0.f, -6360.0 };

			cos_theta = (float)cos(XMConvertToRadians(gui_data.sun_zenith_angle_in_degrees));
			sin_theta = (float)sin(XMConvertToRadians(gui_data.sun_zenith_angle_in_degrees));
			cos_phi = (float)cos(XMConvertToRadians(gui_data.sun_azimuth_angle_in_degrees));
			sin_phi = (float)sin(XMConvertToRadians(gui_data.sun_azimuth_angle_in_degrees));
			cbAtmosphere.m_v->sun_direction_ws = {
				cos_phi * sin_theta,
				sin_phi * sin_theta,
				cos_theta
			};

			// White Balance
			double white_point_r = 1.0;
			double white_point_g = 1.0;
			double white_point_b = 1.0;
			if(gui_data.do_white_balance) {
				atmosphere::compute_white_point(&white_point_r, &white_point_g, &white_point_b);
			}
			cbAtmosphere.m_v->white_point = { (float)white_point_r, (float)white_point_g , (float)white_point_b };

			const double sun_angular_radius_rad = 0.00935 / 2.0;
			cbAtmosphere.m_v->sun_disk_size_x = (float)tan(sun_angular_radius_rad);
			cbAtmosphere.m_v->sun_disk_size_y = (float)cos(sun_angular_radius_rad);
			cbAtmosphere.m_v->exposure = gui_data.exposure;
			cbAtmosphere.Update(renderer, 0);
		}

		//if(gui_data.debug_dump_textures) {
		//	debug_dump_texture_2d(renderer, transmittance_texture, "precomputed_final_transmittance");
		//	debug_dump_texture_2d(renderer, irradiance_texture, "precomputed_final_irradiance");
		//	debug_dump_texture_3d(renderer, scattering_texture, "precomputed_final_scattering");
		//	debug_dump_texture_3d(renderer, single_mie_scattering_texture, "precomputed_final_single_mie_scattering");
		//	gui_data.debug_dump_textures = false;
		//}
	}


	ComPtr<ID3D11DepthStencilState>& get_depth_stencil_state(){
		return com_depth_stencil_state;
	}

	ComPtr<ID3D11BlendState>& get_blend_state_01(){
		return com_blend_state_01;
	}

	ComPtr<ID3D11BlendState>& get_blend_state_0011(){
		return com_blend_state_0011;
	}

	void render_frame(cRenderer &renderer)
	{
		ID3D11DeviceContext *com_device_context = renderer.GetDevContext();

		//float clear_color[4] = { 0.0,1.0,0.0,0.0 };
		//com_device_context->ClearRenderTargetView(com_backbuffer_rtv.Get(), clear_color);
		//com_device_context->ClearRenderTargetView(renderer.m_renderTargetView, clear_color);

		atmosphere::get_demo_vs().Begin();
		atmosphere::get_demo_vs().BeginPass(renderer, 0);

		cbAtmosphere.Update(renderer, 0);

		com_device_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

		renderer.m_viewPort.Bind(renderer);
		
		CommonStates states(renderer.GetDevice());
		com_device_context->RSSetState(states.CullNone());

		ID3D11ShaderResourceView *a_srvs[] = { 
			atmosphere::get_transmittance_texture().m_srv
			, atmosphere::get_scattering_texture().m_srv
			, atmosphere::get_single_mie_scattering_texture().m_srv
			, atmosphere::get_irradiance_texture().m_srv 
		};
		com_device_context->PSSetShaderResources(0, count_of(a_srvs), a_srvs);

		com_device_context->OMSetDepthStencilState(com_depth_stencil_state.Get(), 0);

		com_device_context->Draw(4, 0);

		ID3D11ShaderResourceView *const a_null_srvs[count_of(a_srvs)] = {};
		com_device_context->PSSetShaderResources(0, count_of(a_srvs), a_null_srvs);
	}

	void present_frame() {
		//com_swap_chain->Present(1, 0);
	}

} // namespace renderer