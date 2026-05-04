/*
license.c - client-side license entry and offline validation
Copyright (C) 2026 xash3d adaptation

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
*/

#include "common.h"
#include "client.h"
#include "license.h"

#if XASH_WIN32
#include "platform/win32/net.h"
#elif XASH_PSVITA
#include "platform/psvita/net_psvita.h"
#else
#include "platform/posix/net.h"
#endif

#include <ctype.h>
#include <netdb.h>
#include <sys/time.h>

extern const char *ID_GetMD5( void );

static char s_license_key[256];
static char s_license_status[256] = "no license loaded";
static license_info_t s_license_info;
static const char s_license_token_filename[] = "license.token";
static convar_t *license_server;

static const char s_offline_hmac_secret[] = "xash3d-offline-token-demo-secret-change-me";
static const char s_license_filename[] = "license.key";

static void License_SetStatus( const char *status )
{
    Q_strncpy( s_license_status, status, sizeof( s_license_status ) );
}

const char *License_GetStatus( void )
{
    return s_license_status;
}

const char *License_GetDeviceId( void )
{
    const char *device = ID_GetMD5();
    return device ? device : "unknown-device";
}

void License_SetKey( const char *key )
{
    if( !key )
        return;

    Q_strncpy( s_license_key, key, sizeof( s_license_key ) );
    s_license_key[sizeof( s_license_key ) - 1] = '\0';
    s_license_info.valid = false;
    License_SaveKey();
    License_SetStatus( "license key stored locally" );
}

static void License_SaveKey( void )
{
    if( !s_license_key[0] )
    {
        FS_Delete( s_license_filename );
        return;
    }

    FS_WriteFile( s_license_filename, s_license_key, Q_strlen( s_license_key ) );
}

static void License_SaveOfflineToken( const char *token )
{
    if( !token || !token[0] )
    {
        FS_Delete( s_license_token_filename );
        return;
    }

    FS_WriteFile( s_license_token_filename, token, Q_strlen( token ) );
}

static void License_LoadOfflineToken( void )
{
    int size;
    char *data = (char *)FS_LoadFile( s_license_token_filename, &size, false );

    if( !data || size <= 0 )
        return;

    data[size] = '\0';
    if( !License_ValidateOfflineToken( data ))
    {
        Con_Printf( S_WARN "stored offline token is invalid or expired\n" );
    }
    Mem_Free( data );
}

static void License_LoadKey( void )
{
    int size;
    char *data = (char *)FS_LoadFile( s_license_filename, &size, false );

    if( !data || size <= 0 )
        return;

    Q_strncpy( s_license_key, data, sizeof( s_license_key ) );
    s_license_key[sizeof( s_license_key ) - 1] = '\0';
    Mem_Free( data );
    License_SetStatus( "stored license loaded" );
}

static void License_ClearKey_f( void )
{
    s_license_key[0] = '\0';
    License_SaveKey();
    s_license_info.valid = false;
    License_SetStatus( "license cleared" );
    Con_Printf( "License key cleared.\n" );
}

static void License_SetKey_f( void )
{
    if( Cmd_Argc() != 2 )
    {
        Con_Printf( S_USAGE "license_set <license_key>\n" );
        return;
    }

    Q_strncpy( s_license_key, Cmd_Argv( 1 ), sizeof( s_license_key ) );
    s_license_key[sizeof( s_license_key ) - 1] = '\0';
    License_SaveKey();
    s_license_info.valid = false;
    License_SetStatus( "license key stored locally" );
    Con_Printf( "License key saved. Use license_validate_online to verify against the backend.\n" );
}

static qboolean License_IsValid( void )
{
    return s_license_info.valid;
}

void License_Enforce( void )
{
    if( License_IsValid() )
        return;

    host.allow_console = true;
    if( cls.key_dest != key_console )
        Key_SetKeyDest( key_console );
}

void License_Draw( void )
{
    if( License_IsValid() )
        return;

    if( host.type == HOST_DEDICATED )
        return;

    rgba_t color = { 255, 255, 255, 255 };
    int x = refState.width / 8;
    int y = refState.height / 6;
    Con_DrawString( x, y, "LICENSE VALIDATION REQUIRED", color );
    y += 20;
    Con_DrawString( x, y, "This copy of the game must be verified with your license server.", color );
    y += 20;
    Con_DrawString( x, y, "Set your license key with: license_set <key>", color );
    y += 20;
    Con_DrawString( x, y, "Then validate it with: license_validate_online", color );
    y += 20;
    Con_DrawString( x, y, "If your server URL is different, use: license_validate_online <key> <server_url>", color );
    y += 20;

    if( s_license_key[0] )
    {
        char buffer[256];
        Q_snprintf( buffer, sizeof( buffer ), "stored license key: %s", s_license_key );
        Con_DrawString( x, y, buffer, color );
        y += 20;
    }

    {
        char buffer[256];
        Q_snprintf( buffer, sizeof( buffer ), "device id: %s", License_GetDeviceId() );
        Con_DrawString( x, y, buffer, color );
        y += 20;
    }

    if( license_server && license_server->string[0] )
    {
        char buffer[256];
        Q_snprintf( buffer, sizeof( buffer ), "license server: %s", license_server->string );
        Con_DrawString( x, y, buffer, color );
        y += 20;
    }

    if( s_license_status[0] )
    {
        char buffer[256];
        Q_snprintf( buffer, sizeof( buffer ), "status: %s", s_license_status );
        Con_DrawString( x, y, buffer, color );
    }
}

static qboolean License_ParseServerUrl( const char *url, char *host, int hostSize, int *port, char *path, int pathSize )
{
    if( !url || !host || !port || !path )
        return false;

    const char *src = url;

    if( !Q_strnicmp( src, "http://", 7 ))
        src += 7;
    else if( !Q_strnicmp( src, "https://", 8 ))
    {
        Con_Printf( S_ERROR "HTTPS backend URLs are not supported in this build\n" );
        return false;
    }

    const char *slash = Q_strchr( src, '/' );
    if( slash )
    {
        int hostLen = (int)( slash - src );
        if( hostLen <= 0 || hostLen >= hostSize )
            return false;
        Q_strncpy( host, src, hostLen + 1 );
        Q_strncpy( path, slash, pathSize );
    }
    else
    {
        Q_strncpy( host, src, hostSize );
        Q_strncpy( path, "/", pathSize );
    }

    *port = 80;
    const char *colon = Q_strchr( host, ':' );
    if( colon )
    {
        char portText[8];
        int length = (int)( colon - host );
        if( length <= 0 || length >= hostSize )
            return false;

        Q_strncpy( portText, colon + 1, sizeof( portText ) );
        *port = Q_atoi( portText );
        if( *port <= 0 )
            *port = 80;

        ((char *)colon)[0] = '\0';
    }

    return true;
}

static qboolean License_ParseJsonStringField( const char *json, const char *field, char *dst, int dstSize )
{
    if( !json || !field || !dst )
        return false;

    char pattern[128];
    Q_snprintf( pattern, sizeof( pattern ), "\"%s\"", field );
    const char *pos = Q_stristr( json, pattern );
    if( !pos )
        return false;

    pos = Q_strchr( pos, ':' );
    if( !pos )
        return false;
    pos++;

    while( *pos && isspace( (unsigned char)*pos ))
        pos++;

    if( *pos != '"' )
        return false;
    pos++;

    int index = 0;
    while( *pos && *pos != '"' && index < dstSize - 1 )
    {
        if( *pos == '\\' && pos[1] )
            pos++;
        dst[index++] = *pos++;
    }
    dst[index] = '\0';
    return index > 0;
}

static qboolean License_ParseJsonBoolField( const char *json, const char *field, qboolean *out )
{
    if( !json || !field || !out )
        return false;

    char pattern[128];
    Q_snprintf( pattern, sizeof( pattern ), "\"%s\"", field );
    const char *pos = Q_stristr( json, pattern );
    if( !pos )
        return false;

    pos = Q_strchr( pos, ':' );
    if( !pos )
        return false;
    pos++;

    while( *pos && isspace( (unsigned char)*pos ))
        pos++;

    if( !Q_strnicmp( pos, "true", 4 ))
    {
        *out = true;
        return true;
    }
    if( !Q_strnicmp( pos, "false", 5 ))
    {
        *out = false;
        return true;
    }

    return false;
}

static int License_HttpPost( const char *url, const char *body, char *response, int maxResponse )
{
    char host[128];
    char path[128];
    int port;

    if( !License_ParseServerUrl( url, host, sizeof( host ), &port, path, sizeof( path ) ))
        return -1;

    char portText[16];
    Q_snprintf( portText, sizeof( portText ), "%d", port );

    struct addrinfo hints;
    struct addrinfo *result, *rp;
    Q_memset( &hints, 0, sizeof( hints ) );
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    if( getaddrinfo( host, portText, &hints, &result ) != 0 )
    {
        Con_Printf( S_ERROR "license server name resolution failed\n" );
        return -1;
    }

    int sock = -1;
    for( rp = result; rp != NULL; rp = rp->ai_next )
    {
        sock = socket( rp->ai_family, rp->ai_socktype, rp->ai_protocol );
        if( sock < 0 )
            continue;

        if( connect( sock, rp->ai_addr, rp->ai_addrlen ) == 0 )
            break;

        closesocket( sock );
        sock = -1;
    }
    freeaddrinfo( result );

    if( sock < 0 )
    {
        Con_Printf( S_ERROR "license server connection failed\n" );
        return -1;
    }

    struct timeval timeout;
    timeout.tv_sec = 10;
    timeout.tv_usec = 0;
    setsockopt( sock, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof( timeout ) );
    setsockopt( sock, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof( timeout ) );

    char hostHeader[160];
    if( port != 80 )
        Q_snprintf( hostHeader, sizeof( hostHeader ), "%s:%d", host, port );
    else
        Q_strncpy( hostHeader, host, sizeof( hostHeader ) );

    char request[2048];
    int bodyLength = Q_strlen( body );
    int requestLength = Q_snprintf( request, sizeof( request ),
        "POST %s HTTP/1.1\r\n"
        "Host: %s\r\n"
        "Content-Type: application/json\r\n"
        "Content-Length: %d\r\n"
        "Connection: close\r\n"
        "User-Agent: xash3d-license-client/1.0\r\n"
        "\r\n"
        "%s",
        path, hostHeader, bodyLength, body );

    if( requestLength <= 0 || requestLength >= (int)sizeof( request ))
    {
        closesocket( sock );
        return -1;
    }

    int sent = 0;
    while( sent < requestLength )
    {
        int wrote = send( sock, request + sent, requestLength - sent, 0 );
        if( wrote <= 0 )
        {
            closesocket( sock );
            Con_Printf( S_ERROR "license server send failed\n" );
            return -1;
        }
        sent += wrote;
    }

    int received = 0;
    while( received < maxResponse - 1 )
    {
        int got = recv( sock, response + received, maxResponse - 1 - received, 0 );
        if( got <= 0 )
            break;
        received += got;
    }
    response[received] = '\0';
    closesocket( sock );

    const char *headerEnd = Q_strstr( response, "\r\n\r\n" );
    if( !headerEnd )
        return -1;

    int statusCode = -1;
    if( Q_strnicmp( response, "HTTP/", 5 ) == 0 )
    {
        const char *space = Q_strchr( response, ' ' );
        if( space )
            statusCode = Q_atoi( space + 1 );
    }

    if( headerEnd + 4 < response + received )
        Q_memmove( response, headerEnd + 4, received - (int)( headerEnd + 4 - response ) + 1 );
    else
        response[0] = '\0';

    return statusCode;
}

static qboolean License_ValidateOnline( const char *licenseKey, const char *backendUrl )
{
    char body[512];
    Q_snprintf( body, sizeof( body ), "{\"licenseKey\":\"%s\",\"deviceId\":\"%s\"}", licenseKey, License_GetDeviceId() );

    char response[2048];
    int status = License_HttpPost( backendUrl, body, response, sizeof( response ) );
    if( status < 0 )
        return false;

    qboolean valid = false;
    License_ParseJsonBoolField( response, "valid", &valid );
    if( !valid )
    {
        char message[256];
        if( License_ParseJsonStringField( response, "error", message, sizeof( message )) )
            Con_Printf( S_ERROR "license validation failed: %s\n", message );
        else
            Con_Printf( S_ERROR "license validation failed, server returned status %d\n", status );
        return false;
    }

    char offlineToken[512];
    if( License_ParseJsonStringField( response, "offlineToken", offlineToken, sizeof( offlineToken )) && offlineToken[0] )
    {
        if( !License_ValidateOfflineToken( offlineToken ))
            return false;

        License_SaveOfflineToken( offlineToken );
        License_SetStatus( "online license validated" );
        return true;
    }

    Con_Printf( S_ERROR "license server did not return an offline token\n" );
    return false;
}

static void License_Status_f( void )
{
    Con_Printf( "license status: %s\n", License_GetStatus() );
    if( s_license_key[0] )
        Con_Printf( "stored key: %s\n", s_license_key );
    Con_Printf( "device id: %s\n", License_GetDeviceId() );
    if( s_license_info.valid )
    {
        Con_Printf( "user: %s plan: %s expires: %u\n", s_license_info.user_id, s_license_info.plan, s_license_info.expires );
    }
}

static int License_HexDigit( char ch )
{
    if( ch >= '0' && ch <= '9' )
        return ch - '0';
    if( ch >= 'a' && ch <= 'f' )
        return ch - 'a' + 10;
    if( ch >= 'A' && ch <= 'F' )
        return ch - 'A' + 10;
    return -1;
}

static void License_HexToBytes( const char *hex, byte *out, int maxOut )
{
    int len = Q_strlen( hex );
    int target = Q_min( len / 2, maxOut );

    for( int i = 0; i < target; i++ )
    {
        int hi = License_HexDigit( hex[i * 2] );
        int lo = License_HexDigit( hex[i * 2 + 1] );
        if( hi < 0 || lo < 0 )
            out[i] = 0;
        else
            out[i] = (byte)(( hi << 4 ) | lo);
    }
}

static qboolean License_ReadTokenField( const char *token, const char *field, char *dst, int dstSize )
{
    const char *start = Q_stristr( token, field );
    if( !start )
        return false;

    start += Q_strlen( field );
    if( *start != '=' )
        return false;
    start++;

    const char *end = Q_strchr( start, ';' );
    int length = end ? (int)( end - start ) : Q_strlen( start );
    int copyLen = Q_min( length, dstSize - 1 );

    Q_strncpy( dst, start, copyLen + 1 );
    dst[copyLen] = '\0';
    return copyLen > 0;
}

static void SHA256_Transform( uint32_t state[8], const byte data[64] );

static void SHA256_Init( uint32_t state[8], byte data[64], uint64_t *bitlen )
{
    state[0] = 0x6a09e667;
    state[1] = 0xbb67ae85;
    state[2] = 0x3c6ef372;
    state[3] = 0xa54ff53a;
    state[4] = 0x510e527f;
    state[5] = 0x9b05688c;
    state[6] = 0x1f83d9ab;
    state[7] = 0x5be0cd19;
    *bitlen = 0;
    Q_memset( data, 0, 64 );
}

static uint32_t SHA256_RotateRight( uint32_t value, int bits )
{
    return ( value >> bits ) | ( value << ( 32 - bits ) );
}

static void SHA256_Transform( uint32_t state[8], const byte data[64] )
{
    uint32_t a, b, c, d, e, f, g, h;
    uint32_t m[64];

    for( int i = 0; i < 16; i++ )
    {
        int j = i * 4;
        m[i] = ( data[j] << 24 ) | ( data[j + 1] << 16 ) | ( data[j + 2] << 8 ) | data[j + 3];
    }

    for( int i = 16; i < 64; i++ )
    {
        uint32_t s0 = SHA256_RotateRight( m[i - 15], 7 ) ^ SHA256_RotateRight( m[i - 15], 18 ) ^ ( m[i - 15] >> 3 );
        uint32_t s1 = SHA256_RotateRight( m[i - 2], 17 ) ^ SHA256_RotateRight( m[i - 2], 19 ) ^ ( m[i - 2] >> 10 );
        m[i] = m[i - 16] + s0 + m[i - 7] + s1;
    }

    a = state[0]; b = state[1]; c = state[2]; d = state[3];
    e = state[4]; f = state[5]; g = state[6]; h = state[7];

    static const uint32_t k[64] = {
        0x428a2f98,0x71374491,0xb5c0fbcf,0xe9b5dba5,0x3956c25b,0x59f111f1,0x923f82a4,0xab1c5ed5,
        0xd807aa98,0x12835b01,0x243185be,0x550c7dc3,0x72be5d74,0x80deb1fe,0x9bdc06a7,0xc19bf174,
        0xe49b69c1,0xefbe4786,0x0fc19dc6,0x240ca1cc,0x2de92c6f,0x4a7484aa,0x5cb0a9dc,0x76f988da,
        0x983e5152,0xa831c66d,0xb00327c8,0xbf597fc7,0xc6e00bf3,0xd5a79147,0x06ca6351,0x14292967,
        0x27b70a85,0x2e1b2138,0x4d2c6dfc,0x53380d13,0x650a7354,0x766a0abb,0x81c2c92e,0x92722c85,
        0xa2bfe8a1,0xa81a664b,0xc24b8b70,0xc76c51a3,0xd192e819,0xd6990624,0xf40e3585,0x106aa070,
        0x19a4c116,0x1e376c08,0x2748774c,0x34b0bcb5,0x391c0cb3,0x4ed8aa4a,0x5b9cca4f,0x682e6ff3,
        0x748f82ee,0x78a5636f,0x84c87814,0x8cc70208,0x90befffa,0xa4506ceb,0xbef9a3f7,0xc67178f2
    };

    for( int i = 0; i < 64; i++ )
    {
        uint32_t S1 = SHA256_RotateRight( e, 6 ) ^ SHA256_RotateRight( e, 11 ) ^ SHA256_RotateRight( e, 25 );
        uint32_t ch = ( e & f ) ^ ( ( ~e ) & g );
        uint32_t temp1 = h + S1 + ch + k[i] + m[i];
        uint32_t S0 = SHA256_RotateRight( a, 2 ) ^ SHA256_RotateRight( a, 13 ) ^ SHA256_RotateRight( a, 22 );
        uint32_t maj = ( a & b ) ^ ( a & c ) ^ ( b & c );
        uint32_t temp2 = S0 + maj;

        h = g;
        g = f;
        f = e;
        e = d + temp1;
        d = c;
        c = b;
        b = a;
        a = temp1 + temp2;
    }

    state[0] += a;
    state[1] += b;
    state[2] += c;
    state[3] += d;
    state[4] += e;
    state[5] += f;
    state[6] += g;
    state[7] += h;
}

static void SHA256_Update( uint32_t state[8], byte data[64], uint64_t *bitlen, const byte *input, size_t len )
{
    size_t i;
    size_t index = ( *bitlen / 8 ) % 64;

    *bitlen += len * 8;

    for( i = 0; i < len; i++ )
    {
        data[index++] = input[i];
        if( index == 64 )
        {
            SHA256_Transform( state, data );
            index = 0;
            Q_memset( data, 0, 64 );
        }
    }
}

static uint32_t SHA256_BigEndian32( uint32_t value )
{
    return ((value & 0xFFu) << 24) | ((value & 0xFF00u) << 8) | ((value & 0xFF0000u) >> 8) | ((value & 0xFF000000u) >> 24);
}

static void SHA256_Final( uint32_t state[8], byte data[64], uint64_t bitlen, byte digest[32] )
{
    size_t index = ( bitlen / 8 ) % 64;

    data[index++] = 0x80;
    if( index > 56 )
    {
        while( index < 64 )
            data[index++] = 0x00;
        SHA256_Transform( state, data );
        index = 0;
        Q_memset( data, 0, 64 );
    }

    while( index < 56 )
        data[index++] = 0x00;

    uint32_t hi = SHA256_BigEndian32( (uint32_t)( bitlen >> 32 ) );
    uint32_t lo = SHA256_BigEndian32( (uint32_t)( bitlen & 0xFFFFFFFFu ) );
    Q_memcpy( &data[56], &hi, 4 );
    Q_memcpy( &data[60], &lo, 4 );

    SHA256_Transform( state, data );

    for( int i = 0; i < 8; i++ )
    {
        digest[i * 4 + 0] = ( state[i] >> 24 ) & 0xff;
        digest[i * 4 + 1] = ( state[i] >> 16 ) & 0xff;
        digest[i * 4 + 2] = ( state[i] >> 8 ) & 0xff;
        digest[i * 4 + 3] = state[i] & 0xff;
    }
}

static void HMAC_SHA256( const byte *key, int keyLen, const byte *data, int dataLen, byte out[32] )
{
    byte keyBlock[64];
    byte innerHash[32];
    uint32_t state[8];
    uint64_t bitlen;
    int i;

    Q_memset( keyBlock, 0, sizeof( keyBlock ) );
    if( keyLen > 64 )
    {
        SHA256_Init( state, keyBlock, &bitlen );
        SHA256_Update( state, keyBlock, &bitlen, key, keyLen );
        SHA256_Final( state, keyBlock, bitlen, keyBlock );
        Q_memset( keyBlock + 32, 0, 32 );
    }
    else
    {
        Q_memcpy( keyBlock, key, keyLen );
    }

    for( i = 0; i < 64; i++ )
        keyBlock[i] ^= 0x36;

    SHA256_Init( state, keyBlock, &bitlen );
    SHA256_Update( state, keyBlock, &bitlen, data, dataLen );
    SHA256_Final( state, keyBlock, bitlen, innerHash );

    for( i = 0; i < 64; i++ )
        keyBlock[i] ^= 0x36 ^ 0x5c;

    SHA256_Init( state, keyBlock, &bitlen );
    SHA256_Update( state, keyBlock, &bitlen, innerHash, 32 );
    SHA256_Final( state, keyBlock, bitlen, out );
}

static qboolean License_ParseOfflineToken( const char *token, license_info_t *info, char *payload, int payloadSize, char *signatureHex, int sigSize )
{
    if( !token || !info || !payload || !signatureHex )
        return false;

    const char *sigMarker = Q_stristr( token, ";sig=" );
    if( !sigMarker )
        return false;

    int payloadLen = Q_min( (int)( sigMarker - token ), payloadSize - 1 );
    Q_strncpy( payload, token, payloadLen + 1 );
    payload[payloadLen] = '\0';

    const char *sigStart = sigMarker + 5;
    Q_strncpy( signatureHex, sigStart, sigSize );
    signatureHex[sigSize - 1] = '\0';

    Q_memset( info, 0, sizeof( *info ) );
    info->valid = false;

    if( !License_ReadTokenField( token, "user", info->user_id, sizeof( info->user_id )) )
        return false;
    if( !License_ReadTokenField( token, "plan", info->plan, sizeof( info->plan )) )
        return false;
    if( !License_ReadTokenField( token, "exp", payload, sizeof( payload )) )
        return false;

    {
        uint32_t value = 0;
        for( int i = 0; payload[i]; i++ )
        {
            if( payload[i] < '0' || payload[i] > '9' )
                break;
            value = value * 10 + ( payload[i] - '0' );
        }
        info->expires = value;
    }
    if( !License_ReadTokenField( token, "device", info->device_id, sizeof( info->device_id )) )
        return false;
    if( !License_ReadTokenField( token, "nonce", info->nonce, sizeof( info->nonce )) )
        return false;

    return true;
}

static qboolean License_VerifyOfflineSignature( const char *payload, const char *signatureHex )
{
    byte expected[32];
    byte actual[32];
    HMAC_SHA256( (const byte *)s_offline_hmac_secret, sizeof( s_offline_hmac_secret ) - 1,
                 (const byte *)payload, Q_strlen( payload ), expected );

    int siglen = Q_strlen( signatureHex );
    if( siglen != 64 )
        return false;

    byte sigBytes[32];
    License_HexToBytes( signatureHex, sigBytes, sizeof( sigBytes ) );

    return memcmp( expected, sigBytes, sizeof( expected ) ) == 0;
}

qboolean License_ValidateOfflineToken( const char *token )
{
    char payload[512];
    char signatureHex[128];
    license_info_t parsed;

    if( !License_ParseOfflineToken( token, &parsed, payload, sizeof( payload ), signatureHex, sizeof( signatureHex )) )
    {
        Con_Printf( S_ERROR "invalid offline token format\n" );
        return false;
    }

    if( !License_VerifyOfflineSignature( payload, signatureHex ) )
    {
        Con_Printf( S_ERROR "offline token signature mismatch\n" );
        return false;
    }

    if( parsed.expires < (uint32_t)time( NULL ) )
    {
        Con_Printf( S_ERROR "offline token expired\n" );
        return false;
    }

    if( Q_strcmp( parsed.device_id, License_GetDeviceId() ) )
    {
        Con_Printf( S_ERROR "offline token device id mismatch\n" );
        return false;
    }

    parsed.valid = true;
    s_license_info = parsed;
    License_SaveOfflineToken( token );
    License_SetStatus( "offline token validated" );
    Con_Printf( "offline license validated for %s (%s), expires %u\n", s_license_info.user_id, s_license_info.plan, s_license_info.expires );
    return true;
}

static void License_ValidateOffline_f( void )
{
    if( Cmd_Argc() != 2 )
    {
        Con_Printf( S_USAGE "license_validate_offline <signed_token>\n" );
        return;
    }

    License_ValidateOfflineToken( Cmd_Argv( 1 ) );
}

static qboolean License_ValidateOnline( const char *licenseKey, const char *backendUrl );

static void License_ValidateOnline_f( void )
{
    const char *serverUrl = license_server ? license_server->string : "";
    const char *licenseKey = s_license_key;

    if( Cmd_Argc() == 2 )
    {
        licenseKey = Cmd_Argv( 1 );
    }
    else if( Cmd_Argc() == 3 )
    {
        licenseKey = Cmd_Argv( 1 );
        if( license_server )
            Cvar_Set( "license_server", Cmd_Argv( 2 ) );
        serverUrl = license_server ? license_server->string : "";
    }
    else if( Cmd_Argc() > 3 )
    {
        Con_Printf( S_USAGE "license_validate_online [license_key] [server_url]\n" );
        return;
    }

    if( !licenseKey || !licenseKey[0] )
    {
        Con_Printf( S_ERROR "No license key provided. Use license_set <license_key> first.\n" );
        return;
    }

    if( !serverUrl || !serverUrl[0] )
    {
        Con_Printf( S_ERROR "No license server configured. Set license_server or pass the server URL.\n" );
        return;
    }

    if( License_ValidateOnline( licenseKey, serverUrl ))
    {
        Con_Printf( "Online license validated successfully.\n" );
    }
    else
    {
        Con_Printf( S_ERROR "Online license validation failed.\n" );
    }
}

static void License_Help_f( void )
{
    Con_Printf( "license_set <key>            - store a license key locally\n" );
    Con_Printf( "license_clear               - remove any stored license key\n" );
    Con_Printf( "license_status              - show current license status\n" );
    Con_Printf( "license_validate_offline <token> - validate signed offline license token\n" );
    Con_Printf( "license_validate_online     - explain backend validation path\n" );
    Con_Printf( "license_deviceid            - show current device identifier\n" );
}

static void License_DeviceId_f( void )
{
    Con_Printf( "license device id: %s\n", License_GetDeviceId() );
}

void License_Init( void )
{
    Cmd_AddCommand( "license_set", License_SetKey_f, "store a license key" );
    Cmd_AddCommand( "license_clear", License_ClearKey_f, "clear stored license key" );
    Cmd_AddCommand( "license_status", License_Status_f, "print current license status" );
    Cmd_AddCommand( "license_validate_offline", License_ValidateOffline_f, "validate a signed offline token" );
    Cmd_AddCommand( "license_validate_online", License_ValidateOnline_f, "validate stored license key against the backend" );
    Cmd_AddCommand( "license_help", License_Help_f, "show license commands" );
    Cmd_AddCommand( "license_deviceid", License_DeviceId_f, "print current device identifier" );

    Q_memset( s_license_key, 0, sizeof( s_license_key ) );
    Q_memset( &s_license_info, 0, sizeof( s_license_info ) );
    license_server = Cvar_Get( "license_server", "http://72.60.130.39:3000", FCVAR_ARCHIVE, "License backend server URL" );
    License_LoadKey();
    License_LoadOfflineToken();
}
