//------------------------------------------------------------
// Movement_5_1a.cpp
// ���̂������_���ɑł��o��(%�g�p)
// 
//------------------------------------------------------------

#include <stdlib.h>
#include <math.h>

#define VIEW_WIDTH			640					// ��ʕ�
#define VIEW_HEIGHT			480					// ��ʍ���
#define CHAR_WIDTH			64					// �L�����N�^��
#define CHAR_HEIGHT			64					// �L�����N�^����
#define MAX_BALL_NUM		100					// �ő勅��
#define VEL_WIDTH			10					// ���������x��
#define VEL_HEIGHT			5					// �c�������x��
#define BASE_VEL			17.0f				// ��b���x
#define GR					0.5f				// �d�͉����x


struct BALL {
	int				bAvctive;					// �L�����ǂ���
	float			x, y;						// �ʒu
	float			vx, vy;						// ���x
};


BALL				Balls[MAX_BALL_NUM];		// ���o�b�t�@
int					nBallNum;					// ���̐�

int					nTimeCount;					// ����


int InitCharacter( void )						// �ŏ��ɂP�񂾂��Ă΂��
{
	int					i;
	// �S�Ă̋��𖳌���
	for ( i = 0; i < MAX_BALL_NUM; i++ ) {
		Balls[i].bAvctive = false;
	}
	nBallNum = 0;								// �e�̐��̓[��

	nTimeCount = 0;

	return 0;
}


int MoveCharacter( void )						// ���t���[���Ă΂��
{
	int					i;

	// ���̈ړ�
	for ( i = 0; i < MAX_BALL_NUM; i++ ) {
		if ( Balls[i].bAvctive ) {
			Balls[i].x  += Balls[i].vx;
			Balls[i].y  += Balls[i].vy;
			Balls[i].vy += GR;
			// ��ʊO�ɏo��Ζ�����
			if ( ( Balls[i].x < -CHAR_WIDTH )  || ( Balls[i].x > VIEW_WIDTH  ) ||
				 ( Balls[i].y < -CHAR_HEIGHT ) || ( Balls[i].y > VIEW_HEIGHT ) )
			{
				Balls[i].bAvctive = false;
			}
		}
	}

	// ���̔���
	if ( ( nTimeCount % 2 ) == 0 ) {
		for ( i = 0; i < MAX_BALL_NUM; i++ ) {
			if ( Balls[i].bAvctive == false ) {
				Balls[i].bAvctive = true;
				Balls[i].x = ( VIEW_WIDTH - CHAR_WIDTH ) / 2.0f;		// ��ʂ̐^�񒆂�
				Balls[i].y = VIEW_HEIGHT - CHAR_HEIGHT;					// ��ʂ̉��[��
				Balls[i].vx = ( rand() % ( VEL_WIDTH  + 1 ) )
							  - VEL_WIDTH  / 2.0f;
				Balls[i].vy = ( rand() % ( VEL_HEIGHT + 1 ) )
							  - VEL_HEIGHT / 2.0f - BASE_VEL;
				break;
			}
		}
	}
	nTimeCount++;

	return 0;
}


//------------------------------------------------------------
// �ȉ��ADirectX�ɂ��\���v���O����

#include <stdio.h>
#include <windows.h>
#include <tchar.h>								// Unicode�E�}���`�o�C�g�����֌W

#include <D3D11.h>
#include <D3DX11.h>
#include <D3Dcompiler.h>
#include <xnamath.h>


#define MAX_BUFFER_VERTEX				1000	// �ő�o�b�t�@���_��


// �����N���C�u����
#pragma comment( lib, "d3d11.lib" )   // D3D11���C�u����
#pragma comment( lib, "d3dx11.lib" )


// �Z�[�t�����[�X�}�N��
#ifndef SAFE_RELEASE
#define SAFE_RELEASE( p )      { if ( p ) { ( p )->Release(); ( p )=NULL; } }
#endif


// ���_�\����
struct CUSTOMVERTEX {
    XMFLOAT4	v4Pos;
    XMFLOAT4	v4Color;
	XMFLOAT2	v2UV;
};

// �V�F�[�_�萔�\����
struct CBNeverChanges
{
    XMMATRIX mView;
};

// �e�N�X�`���G�\����
struct TEX_PICTURE {
	ID3D11ShaderResourceView	*pSRViewTexture;
	D3D11_TEXTURE2D_DESC		tdDesc;
	int							nWidth, nHeight;
};


// �O���[�o���ϐ�
UINT  g_nClientWidth;							// �`��̈�̉���
UINT  g_nClientHeight;							// �`��̈�̍���

HWND        g_hWnd;         // �E�B���h�E�n���h��


ID3D11Device			*g_pd3dDevice;			// �f�o�C�X
IDXGISwapChain			*g_pSwapChain;			// DXGI�X���b�v�`�F�C��
ID3D11DeviceContext		*g_pImmediateContext;	// �f�o�C�X�R���e�L�X�g
ID3D11RasterizerState	*g_pRS;					// ���X�^���C�U
ID3D11RenderTargetView	*g_pRTV;				// �����_�����O�^�[�Q�b�g
D3D_FEATURE_LEVEL       g_FeatureLevel;			// �t�B�[�`���[���x��

ID3D11Buffer			*g_pD3D11VertexBuffer;
ID3D11BlendState		*g_pbsAlphaBlend;
ID3D11VertexShader		*g_pVertexShader;
ID3D11PixelShader		*g_pPixelShader;
ID3D11InputLayout		*g_pInputLayout;
ID3D11SamplerState		*g_pSamplerState;

ID3D11Buffer			*g_pCBNeverChanges = NULL;

TEX_PICTURE				g_tBall, g_tBack;

// �`�撸�_�o�b�t�@
CUSTOMVERTEX g_cvVertices[MAX_BUFFER_VERTEX];
int							g_nVertexNum = 0;
ID3D11ShaderResourceView	*g_pNowTexture = NULL;


// Direct3D�̏�����
HRESULT InitD3D( void )
{
    HRESULT hr = S_OK;
	D3D_FEATURE_LEVEL  FeatureLevelsRequested[6] = { D3D_FEATURE_LEVEL_11_0,
													 D3D_FEATURE_LEVEL_10_1,
													 D3D_FEATURE_LEVEL_10_0,
													 D3D_FEATURE_LEVEL_9_3,
													 D3D_FEATURE_LEVEL_9_2,
													 D3D_FEATURE_LEVEL_9_1 };
	UINT               numLevelsRequested = 6;
	D3D_FEATURE_LEVEL  FeatureLevelsSupported;

	// �f�o�C�X�쐬
	hr = D3D11CreateDevice( NULL,
					D3D_DRIVER_TYPE_HARDWARE, 
					NULL, 
					0,
					FeatureLevelsRequested, 
					numLevelsRequested,
					D3D11_SDK_VERSION, 
					&g_pd3dDevice,
					&FeatureLevelsSupported,
					&g_pImmediateContext );
	if( FAILED ( hr ) ) {
		return hr;
	}

	// �t�@�N�g���̎擾
	IDXGIDevice * pDXGIDevice;
	hr = g_pd3dDevice->QueryInterface( __uuidof( IDXGIDevice ), ( void ** )&pDXGIDevice );
	IDXGIAdapter * pDXGIAdapter;
	hr = pDXGIDevice->GetParent( __uuidof( IDXGIAdapter ), ( void ** )&pDXGIAdapter );
	IDXGIFactory * pIDXGIFactory;
	pDXGIAdapter->GetParent( __uuidof( IDXGIFactory ), ( void ** )&pIDXGIFactory);

	// �X���b�v�`�F�C���̍쐬
    DXGI_SWAP_CHAIN_DESC	sd;
	ZeroMemory( &sd, sizeof( sd ) );
	sd.BufferCount = 1;
	sd.BufferDesc.Width = g_nClientWidth;
	sd.BufferDesc.Height = g_nClientHeight;
	sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	sd.BufferDesc.RefreshRate.Numerator = 60;
	sd.BufferDesc.RefreshRate.Denominator = 1;
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.OutputWindow = g_hWnd;
	sd.SampleDesc.Count = 1;
	sd.SampleDesc.Quality = 0;
	sd.Windowed = TRUE;
	hr = pIDXGIFactory->CreateSwapChain( g_pd3dDevice, &sd, &g_pSwapChain );

	pDXGIDevice->Release();
	pDXGIAdapter->Release();
	pIDXGIFactory->Release();

	if( FAILED ( hr ) ) {
		return hr;
	}

    // �����_�����O�^�[�Q�b�g�̐���
    ID3D11Texture2D			*pBackBuffer = NULL;
    D3D11_TEXTURE2D_DESC BackBufferSurfaceDesc;
    hr = g_pSwapChain->GetBuffer( 0, __uuidof( ID3D11Texture2D ), ( LPVOID* )&pBackBuffer );
    if ( FAILED( hr ) ) {
		MessageBox( NULL, _T( "Can't get backbuffer." ), _T( "Error" ), MB_OK );
        return hr;
    }
    pBackBuffer->GetDesc( &BackBufferSurfaceDesc );
    hr = g_pd3dDevice->CreateRenderTargetView( pBackBuffer, NULL, &g_pRTV );
    SAFE_RELEASE( pBackBuffer );
    if ( FAILED( hr ) ) {
		MessageBox( NULL, _T( "Can't create render target view." ), _T( "Error" ), MB_OK );
        return hr;
    }

    g_pImmediateContext->OMSetRenderTargets( 1, &g_pRTV, NULL );

    // ���X�^���C�U�̐ݒ�
    D3D11_RASTERIZER_DESC drd;
	ZeroMemory( &drd, sizeof( drd ) );
	drd.FillMode				= D3D11_FILL_SOLID;
	drd.CullMode				= D3D11_CULL_NONE;
	drd.FrontCounterClockwise	= FALSE;
	drd.DepthClipEnable			= TRUE;
    hr = g_pd3dDevice->CreateRasterizerState( &drd, &g_pRS );
    if ( FAILED( hr ) ) {
		MessageBox( NULL, _T( "Can't create rasterizer state." ), _T( "Error" ), MB_OK );
        return hr;
    }
    g_pImmediateContext->RSSetState( g_pRS );

    // �r���[�|�[�g�̐ݒ�
    D3D11_VIEWPORT vp;
    vp.Width    = ( FLOAT )g_nClientWidth;
    vp.Height   = ( FLOAT )g_nClientHeight;
    vp.MinDepth = 0.0f;
    vp.MaxDepth = 1.0f;
    vp.TopLeftX = 0.0f;
    vp.TopLeftY = 0.0f;
    g_pImmediateContext->RSSetViewports( 1, &vp );

    return S_OK;
}


// �v���O���}�u���V�F�[�_�쐬
HRESULT MakeShaders( void )
{
    HRESULT hr;
    ID3DBlob* pVertexShaderBuffer = NULL;
    ID3DBlob* pPixelShaderBuffer = NULL;
    ID3DBlob* pError = NULL;

    DWORD dwShaderFlags = 0;
#ifdef _DEBUG
    dwShaderFlags |= D3DCOMPILE_DEBUG;
#endif
    // �R���p�C��
    hr = D3DX11CompileFromFile( _T( "Basic_2D.fx" ), NULL, NULL, "VS", "vs_4_0_level_9_1",
								dwShaderFlags, 0, NULL, &pVertexShaderBuffer, &pError, NULL );
    if ( FAILED( hr ) ) {
		MessageBox( NULL, _T( "Can't open Basic_2D.fx" ), _T( "Error" ), MB_OK );
        SAFE_RELEASE( pError );
        return hr;
    }
    hr = D3DX11CompileFromFile( _T( "Basic_2D.fx" ), NULL, NULL, "PS", "ps_4_0_level_9_1",
								dwShaderFlags, 0, NULL, &pPixelShaderBuffer, &pError, NULL );
    if ( FAILED( hr ) ) {
        SAFE_RELEASE( pVertexShaderBuffer );
        SAFE_RELEASE( pError );
        return hr;
    }
    SAFE_RELEASE( pError );
    
    // VertexShader�쐬
    hr = g_pd3dDevice->CreateVertexShader( pVertexShaderBuffer->GetBufferPointer(),
										   pVertexShaderBuffer->GetBufferSize(),
										   NULL, &g_pVertexShader );
    if ( FAILED( hr ) ) {
        SAFE_RELEASE( pVertexShaderBuffer );
        SAFE_RELEASE( pPixelShaderBuffer );
        return hr;
    }
    // PixelShader�쐬
    hr = g_pd3dDevice->CreatePixelShader( pPixelShaderBuffer->GetBufferPointer(),
										  pPixelShaderBuffer->GetBufferSize(),
										  NULL, &g_pPixelShader );
    if ( FAILED( hr ) ) {
        SAFE_RELEASE( pVertexShaderBuffer );
        SAFE_RELEASE( pPixelShaderBuffer );
        return hr;
    }

    // ���̓o�b�t�@�̓��͌`��
    D3D11_INPUT_ELEMENT_DESC layout[] = {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0,  0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "COLOR",    0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 16, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TEXTURE",  0, DXGI_FORMAT_R32G32_FLOAT,       0, 32, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };
	UINT numElements = ARRAYSIZE( layout );
	// ���̓o�b�t�@�̓��͌`���쐬
    hr = g_pd3dDevice->CreateInputLayout( layout, numElements,
										  pVertexShaderBuffer->GetBufferPointer(),
										  pVertexShaderBuffer->GetBufferSize(),
										  &g_pInputLayout );
    SAFE_RELEASE( pVertexShaderBuffer );
    SAFE_RELEASE( pPixelShaderBuffer );
    if ( FAILED( hr ) ) {
        return hr;
    }

    // �V�F�[�_�萔�o�b�t�@�쐬
    D3D11_BUFFER_DESC bd;
    ZeroMemory( &bd, sizeof( bd ) );
    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.ByteWidth = sizeof( CBNeverChanges );
    bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    bd.CPUAccessFlags = 0;
    hr = g_pd3dDevice->CreateBuffer( &bd, NULL, &g_pCBNeverChanges );
    if( FAILED( hr ) )
        return hr;

	// �ϊ��s��
    CBNeverChanges	cbNeverChanges;
	XMMATRIX		mScreen;
    mScreen = XMMatrixIdentity();
	mScreen._11 =  2.0f / g_nClientWidth;
	mScreen._22 = -2.0f / g_nClientHeight;
	mScreen._41 = -1.0f;
	mScreen._42 =  1.0f;
	cbNeverChanges.mView = XMMatrixTranspose( mScreen );
	g_pImmediateContext->UpdateSubresource( g_pCBNeverChanges, 0, NULL, &cbNeverChanges, 0, 0 );

    return S_OK;
}


// �e�N�X�`�����[�h
int LoadTexture( TCHAR *szFileName, TEX_PICTURE *pTexPic, int nWidth, int nHeight,
				 int nTexWidth, int nTexHeight )
{
    HRESULT						hr;
	D3DX11_IMAGE_LOAD_INFO		liLoadInfo;
	ID3D11Texture2D				*pTexture;

	ZeroMemory( &liLoadInfo, sizeof( D3DX11_IMAGE_LOAD_INFO ) );
	liLoadInfo.Width = nTexWidth;
	liLoadInfo.Height = nTexHeight;
	liLoadInfo.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	liLoadInfo.Format = DXGI_FORMAT_R8G8B8A8_UNORM;

	hr = D3DX11CreateShaderResourceViewFromFile( g_pd3dDevice, szFileName, &liLoadInfo,
												 NULL, &( pTexPic->pSRViewTexture ), NULL );
    if ( FAILED( hr ) ) {
        return hr;
    }
	pTexPic->pSRViewTexture->GetResource( ( ID3D11Resource ** )&( pTexture ) );
	pTexture->GetDesc( &( pTexPic->tdDesc ) );
	pTexture->Release();

	pTexPic->nWidth = nWidth;
	pTexPic->nHeight = nHeight;

	return S_OK;
}


// �`�惂�[�h�I�u�W�F�N�g������
int InitDrawModes( void )
{
    HRESULT				hr;

	// �u�����h�X�e�[�g
    D3D11_BLEND_DESC BlendDesc;
	BlendDesc.AlphaToCoverageEnable = FALSE;
	BlendDesc.IndependentBlendEnable = FALSE;
    BlendDesc.RenderTarget[0].BlendEnable           = TRUE;
    BlendDesc.RenderTarget[0].SrcBlend              = D3D11_BLEND_SRC_ALPHA;
    BlendDesc.RenderTarget[0].DestBlend             = D3D11_BLEND_INV_SRC_ALPHA;
    BlendDesc.RenderTarget[0].BlendOp               = D3D11_BLEND_OP_ADD;
    BlendDesc.RenderTarget[0].SrcBlendAlpha         = D3D11_BLEND_ONE;
    BlendDesc.RenderTarget[0].DestBlendAlpha        = D3D11_BLEND_ZERO;
    BlendDesc.RenderTarget[0].BlendOpAlpha          = D3D11_BLEND_OP_ADD;
    BlendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
    hr = g_pd3dDevice->CreateBlendState( &BlendDesc, &g_pbsAlphaBlend );
    if ( FAILED( hr ) ) {
        return hr;
    }

    // �T���v��
    D3D11_SAMPLER_DESC samDesc;
    ZeroMemory( &samDesc, sizeof( samDesc ) );
    samDesc.Filter          = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    samDesc.AddressU        = D3D11_TEXTURE_ADDRESS_WRAP;
    samDesc.AddressV        = D3D11_TEXTURE_ADDRESS_WRAP;
    samDesc.AddressW        = D3D11_TEXTURE_ADDRESS_WRAP;
    samDesc.ComparisonFunc  = D3D11_COMPARISON_ALWAYS;
    samDesc.MaxLOD          = D3D11_FLOAT32_MAX;
    hr = g_pd3dDevice->CreateSamplerState( &samDesc, &g_pSamplerState );
    if ( FAILED( hr ) ) {
        return hr;
    }

    return S_OK;
}


// �W�I���g���̏�����
HRESULT InitGeometry( void )
{
    HRESULT hr = S_OK;

    // ���_�o�b�t�@�쐬
    D3D11_BUFFER_DESC BufferDesc;
    BufferDesc.Usage                = D3D11_USAGE_DYNAMIC;
    BufferDesc.ByteWidth            = sizeof( CUSTOMVERTEX ) * MAX_BUFFER_VERTEX;
    BufferDesc.BindFlags            = D3D11_BIND_VERTEX_BUFFER;
    BufferDesc.CPUAccessFlags       = D3D11_CPU_ACCESS_WRITE;
    BufferDesc.MiscFlags            = 0;

    D3D11_SUBRESOURCE_DATA SubResourceData;
    SubResourceData.pSysMem             = g_cvVertices;
    SubResourceData.SysMemPitch         = 0;
    SubResourceData.SysMemSlicePitch    = 0;
    hr = g_pd3dDevice->CreateBuffer( &BufferDesc, &SubResourceData, &g_pD3D11VertexBuffer );
    if ( FAILED( hr ) ) {
        return hr;
    }

	// �e�N�X�`���쐬
	g_tBall.pSRViewTexture =  NULL;
	g_tBack.pSRViewTexture =  NULL;
	hr = LoadTexture( _T( "3.dds" ), &g_tBall, 64, 64, 64, 64 );
    if ( FAILED( hr ) ) {
 		MessageBox( NULL, _T( "Can't open 3.dds" ), _T( "Error" ), MB_OK );
       return hr;
    }
	hr = LoadTexture( _T( "4.bmp" ), &g_tBack, 640, 480, 1024, 512 );
	if ( FAILED( hr ) ) {
 		MessageBox( NULL, _T( "Can't open 4.BMP" ), _T( "Error" ), MB_OK );
        return hr;
    }

	return S_OK;
}


// �I������
int Cleanup( void )
{
    SAFE_RELEASE( g_tBall.pSRViewTexture );
    SAFE_RELEASE( g_tBack.pSRViewTexture );
    SAFE_RELEASE( g_pD3D11VertexBuffer );

    SAFE_RELEASE( g_pSamplerState );
    SAFE_RELEASE( g_pbsAlphaBlend );
    SAFE_RELEASE( g_pInputLayout );
    SAFE_RELEASE( g_pPixelShader );
    SAFE_RELEASE( g_pVertexShader );
    SAFE_RELEASE( g_pCBNeverChanges );

    SAFE_RELEASE( g_pRS );									// ���X�^���C�U

	// �X�e�[�^�X���N���A
	if ( g_pImmediateContext ) {
		g_pImmediateContext->ClearState();
		g_pImmediateContext->Flush();
	}

    SAFE_RELEASE( g_pRTV );									// �����_�����O�^�[�Q�b�g

    // �X���b�v�`�F�[��
    if ( g_pSwapChain != NULL ) {
        g_pSwapChain->SetFullscreenState( FALSE, 0 );
    }
    SAFE_RELEASE( g_pSwapChain );

    SAFE_RELEASE( g_pImmediateContext );					// �f�o�C�X�R���e�L�X�g
    SAFE_RELEASE( g_pd3dDevice );							// �f�o�C�X

	return 0;
}


// �E�B���h�E�v���V�[�W��
LRESULT WINAPI MsgProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam )
{
    switch( msg )
    {
        case WM_DESTROY:
            PostQuitMessage( 0 );
            return 0;
    }

    return DefWindowProc( hWnd, msg, wParam, lParam );
}


// �G�̕`��҂��s��t���b�V��
int FlushDrawingPictures( void )
{
	HRESULT			hr;

	if ( ( g_nVertexNum > 0 ) && g_pNowTexture ) {
		D3D11_MAPPED_SUBRESOURCE mappedResource;
		hr = g_pImmediateContext->Map( g_pD3D11VertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource );
		if ( SUCCEEDED( hr ) ) {
			CopyMemory( mappedResource.pData, &( g_cvVertices[0] ), sizeof( CUSTOMVERTEX ) * g_nVertexNum );
			g_pImmediateContext->Unmap( g_pD3D11VertexBuffer, 0 );
		}
		g_pImmediateContext->PSSetShaderResources( 0, 1, &g_pNowTexture );
		g_pImmediateContext->Draw( g_nVertexNum, 0 );
	}
	g_nVertexNum = 0;
	g_pNowTexture = NULL;

	return 0;
}


// �G�̕`��
int DrawPicture( float x, float y, TEX_PICTURE *pTexPic )
{
	if ( g_nVertexNum > ( MAX_BUFFER_VERTEX - 6 ) ) return -1;	// ���_���o�b�t�@���炠�ӂ��ꍇ�͕`�悹��

	// �e�N�X�`�����؂�ւ����Ă���Α҂��s��t���b�V��
	if ( ( pTexPic->pSRViewTexture != g_pNowTexture ) && g_pNowTexture ) {
		FlushDrawingPictures();
	}

	// ���_�Z�b�g
	g_cvVertices[g_nVertexNum + 0].v4Pos   = XMFLOAT4( x,                         y,                          0.0f, 1.0f );
	g_cvVertices[g_nVertexNum + 0].v4Color = XMFLOAT4( 1.0f, 1.0f, 1.0f, 1.0f );
	g_cvVertices[g_nVertexNum + 0].v2UV    = XMFLOAT2( 0.0f, 0.0f );
	g_cvVertices[g_nVertexNum + 1].v4Pos   = XMFLOAT4( x,                         y + pTexPic->nHeight, 0.0f, 1.0f );
	g_cvVertices[g_nVertexNum + 1].v4Color = XMFLOAT4( 1.0f, 1.0f, 1.0f, 1.0f );
	g_cvVertices[g_nVertexNum + 1].v2UV    = XMFLOAT2( 0.0f, 1.0f );
	g_cvVertices[g_nVertexNum + 2].v4Pos   = XMFLOAT4( x + pTexPic->nWidth, y,                          0.0f, 1.0f );
	g_cvVertices[g_nVertexNum + 2].v4Color = XMFLOAT4( 1.0f, 1.0f, 1.0f, 1.0f );
	g_cvVertices[g_nVertexNum + 2].v2UV    = XMFLOAT2( 1.0f, 0.0f );
	g_cvVertices[g_nVertexNum + 3] = g_cvVertices[g_nVertexNum + 1];
	g_cvVertices[g_nVertexNum + 4].v4Pos   = XMFLOAT4( x + pTexPic->nWidth, y + pTexPic->nHeight, 0.0f, 1.0f );
	g_cvVertices[g_nVertexNum + 4].v4Color = XMFLOAT4( 1.0f, 1.0f, 1.0f, 1.0f );
	g_cvVertices[g_nVertexNum + 4].v2UV    = XMFLOAT2( 1.0f, 1.0f );
	g_cvVertices[g_nVertexNum + 5] = g_cvVertices[g_nVertexNum + 2];
	g_nVertexNum += 6;
	g_pNowTexture = pTexPic->pSRViewTexture;

	return 0;
}


// �����_�����O
HRESULT Render( void )
{
    HRESULT			hr = S_OK;
	int				i;

    // ��ʃN���A
    float color[] = { 0.0f, 0.0f, 1.0f, 1.0f };
    g_pImmediateContext->ClearRenderTargetView( g_pRTV, color );

    // �T���v���E���X�^���C�U�Z�b�g
    g_pImmediateContext->PSSetSamplers( 0, 1, &g_pSamplerState );
    g_pImmediateContext->RSSetState( g_pRS );
    
    // �`��ݒ�
    UINT nStrides = sizeof( CUSTOMVERTEX );
    UINT nOffsets = 0;
    g_pImmediateContext->IASetVertexBuffers( 0, 1, &g_pD3D11VertexBuffer, &nStrides, &nOffsets );
    g_pImmediateContext->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST );
    g_pImmediateContext->IASetInputLayout( g_pInputLayout );

    // �V�F�[�_�ݒ�
    g_pImmediateContext->VSSetShader( g_pVertexShader, NULL, 0 );
    g_pImmediateContext->VSSetConstantBuffers( 0, 1, &g_pCBNeverChanges );
    g_pImmediateContext->PSSetShader( g_pPixelShader, NULL, 0 );

    // �`��
    g_pImmediateContext->OMSetBlendState( NULL, NULL, 0xFFFFFFFF );
	DrawPicture( 0.0f, 0.0f, &g_tBack );
    g_pImmediateContext->OMSetBlendState( g_pbsAlphaBlend, NULL, 0xFFFFFFFF );
	for ( i = 0; i < MAX_BALL_NUM; i++ ) {
		if ( Balls[i].bAvctive ) {
			DrawPicture( Balls[i].x, Balls[i].y, &g_tBall );
		}
	}

    // �\��
	FlushDrawingPictures();

    return S_OK;
}


// �G���g���|�C���g
int WINAPI _tWinMain( HINSTANCE hInst, HINSTANCE, LPTSTR, int )
{
	LARGE_INTEGER			nNowTime, nLastTime;		// ���݂ƂЂƂO�̎���
	LARGE_INTEGER			nTimeFreq;					// ���ԒP��

    // ��ʃT�C�Y
    g_nClientWidth  = VIEW_WIDTH;						// ��
    g_nClientHeight = VIEW_HEIGHT;						// ����

	// Register the window class
    WNDCLASSEX wc = { sizeof( WNDCLASSEX ), CS_CLASSDC, MsgProc, 0L, 0L,
                      GetModuleHandle( NULL ), NULL, NULL, NULL, NULL,
                      _T( "D3D Sample" ), NULL };
    RegisterClassEx( &wc );

	RECT rcRect;
	SetRect( &rcRect, 0, 0, g_nClientWidth, g_nClientHeight );
	AdjustWindowRect( &rcRect, WS_OVERLAPPEDWINDOW, FALSE );
    g_hWnd = CreateWindow( _T( "D3D Sample" ), _T( "Movement_5_1a" ),
						   WS_OVERLAPPEDWINDOW, 100, 20, rcRect.right - rcRect.left, rcRect.bottom - rcRect.top,
						   GetDesktopWindow(), NULL, wc.hInstance, NULL );

    // Initialize Direct3D
    if( SUCCEEDED( InitD3D() ) && SUCCEEDED( MakeShaders() ) )
    {
        // Create the shaders
        if( SUCCEEDED( InitDrawModes() ) )
        {
			if ( SUCCEEDED( InitGeometry() ) ) {					// �W�I���g���쐬

				// Show the window
				ShowWindow( g_hWnd, SW_SHOWDEFAULT );
				UpdateWindow( g_hWnd );

				InitCharacter();									// �L�����N�^������
				
				QueryPerformanceFrequency( &nTimeFreq );			// ���ԒP��
				QueryPerformanceCounter( &nLastTime );				// 1�t���[���O����������

				// Enter the message loop
				MSG msg;
				ZeroMemory( &msg, sizeof( msg ) );
				while( msg.message != WM_QUIT )
				{
					Render();
					MoveCharacter();
					do {
						if( PeekMessage( &msg, NULL, 0U, 0U, PM_REMOVE ) )
						{
							TranslateMessage( &msg );
							DispatchMessage( &msg );
						}
						QueryPerformanceCounter( &nNowTime );
					} while( ( ( nNowTime.QuadPart - nLastTime.QuadPart ) < ( nTimeFreq.QuadPart / 90 ) ) &&
							 ( msg.message != WM_QUIT ) );
					while( ( ( nNowTime.QuadPart - nLastTime.QuadPart ) < ( nTimeFreq.QuadPart / 60 ) ) &&
						   ( msg.message != WM_QUIT ) )
					{
						QueryPerformanceCounter( &nNowTime );
					}
					nLastTime = nNowTime;
					g_pSwapChain->Present( 0, 0 );					// �\��
				}
			}
        }
    }

    // Clean up everything and exit the app
    Cleanup();
    UnregisterClass( _T( "D3D Sample" ), wc.hInstance );
    return 0;
}