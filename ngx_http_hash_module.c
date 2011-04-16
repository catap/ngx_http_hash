
/*
 * Copyright (C) Kirill A. Korinskiy
 */


#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_http.h>
#include <ngx_md5.h>


typedef struct {
    ngx_int_t    start;
    ngx_int_t    end;
    ngx_http_complex_value_t    value;
} ngx_http_hash_ctx_t;

static char *ngx_http_set_md5_hash_by_slot(ngx_conf_t *cf,
    ngx_command_t *cmd, void *conf);
static char *ngx_http_set_crc32_hash_by_slot(ngx_conf_t *cf,
    ngx_command_t *cmd, void *conf);
static char *ngx_http_set_lookup3_hash_by_slot(ngx_conf_t *cf,
    ngx_command_t *cmd, void *conf);

static ngx_command_t  ngx_http_hash_commands[] = {

    { ngx_string("md5_hash"),
      NGX_HTTP_MAIN_CONF|NGX_HTTP_SRV_CONF|NGX_HTTP_LOC_CONF|NGX_CONF_TAKE23,
      ngx_http_set_md5_hash_by_slot,
      NGX_HTTP_LOC_CONF_OFFSET,
      0,
      NULL },

    { ngx_string("crc32_hash"),
      NGX_HTTP_MAIN_CONF|NGX_HTTP_SRV_CONF|NGX_HTTP_LOC_CONF|NGX_CONF_TAKE23,
      ngx_http_set_crc32_hash_by_slot,
      NGX_HTTP_LOC_CONF_OFFSET,
      0,
      NULL },

#if (NGX_HAVE_LOOKUP3)
    { ngx_string("lookup3_hash"),
      NGX_HTTP_MAIN_CONF|NGX_HTTP_SRV_CONF|NGX_HTTP_LOC_CONF|NGX_CONF_TAKE23,
      ngx_http_set_lookup3_hash_by_slot,
      NGX_HTTP_LOC_CONF_OFFSET,
      0,
      NULL },
#endif

      ngx_null_command
};


static ngx_http_module_t  ngx_http_hash_module_ctx = {
    NULL,                                /* preconfiguration */
    NULL,                                /* postconfiguration */

    NULL,                                /* create main configuration */
    NULL,                                /* init main configuration */

    NULL,                                /* create server configuration */
    NULL,                                /* merge server configuration */

    NULL,                                /* create location configuration */
    NULL                                 /* merge location configuration */
};


ngx_module_t  ngx_http_hash_module = {
    NGX_MODULE_V1,
    &ngx_http_hash_module_ctx,             /* module context */
    ngx_http_hash_commands,                /* module directives */
    NGX_HTTP_MODULE,                       /* module type */
    NULL,                                  /* init master */
    NULL,                                  /* init module */
    NULL,                                  /* init process */
    NULL,                                  /* init thread */
    NULL,                                  /* exit thread */
    NULL,                                  /* exit process */
    NULL,                                  /* exit master */
    NGX_MODULE_V1_PADDING
};


static ngx_int_t
ngx_http_crc32_hash_variable(ngx_http_request_t *r, ngx_http_variable_value_t *v,
    uintptr_t data)
{
    ngx_http_hash_ctx_t *ctx = (ngx_http_hash_ctx_t *)data;

    ngx_str_t                   val;

    if (ngx_http_complex_value(r, &ctx->value, &val) != NGX_OK) {
        return NGX_ERROR;
    }

    v->len = sizeof("3C32515A") - 1;

    if (v->len <= ctx->start) {
        return NGX_ERROR;
    }

    if (v->len < ctx->end) {
        return NGX_ERROR;
    }

    v->data = ngx_palloc(r->pool, v->len);
    if (v->data == NULL) {
        v->not_found = 1;
        return NGX_OK;
    }

    ngx_sprintf(v->data, "%08XD", ngx_crc32_long(val.data, val.len));

    v->valid = 1;
    v->not_found = 0;
    v->no_cacheable = 0;

    if (ctx->end) {
        v->data += ctx->start;
        v->len = ctx->end;
    }

    return NGX_OK;
}


static ngx_int_t
ngx_http_md5_hash_variable(ngx_http_request_t *r, ngx_http_variable_value_t *v,
    uintptr_t data)
{
    ngx_http_hash_ctx_t *ctx = (ngx_http_hash_ctx_t *)data;

    ngx_uint_t i;

    ngx_str_t  val;
    ngx_md5_t  md5;
    u_char     hash[16], *p;

    if (ngx_http_complex_value(r, &ctx->value, &val) != NGX_OK) {
        return NGX_ERROR;
    }

    v->len = sizeof("7002945D4B8D9E472866092689DB3EAD") - 1;

    if (v->len <= ctx->start) {
        return NGX_ERROR;
    }

    if (v->len < ctx->end) {
        return NGX_ERROR;
    }

    v->data = ngx_palloc(r->pool, v->len);
    if (v->data == NULL) {
        v->not_found = 1;
        return NGX_OK;
    }

    ngx_md5_init(&md5);
    ngx_md5_update(&md5, val.data, val.len);
    ngx_md5_final((u_char*)hash, &md5);

    p = v->data;

    for (i = 0; i < 16; i++) {
        p = ngx_sprintf(p, "%02XD", hash[i]);
    }

    v->valid = 1;
    v->not_found = 0;
    v->no_cacheable = 0;

    if (ctx->end) {
        v->data += ctx->start;
        v->len = ctx->end;
    }


    return NGX_OK;
}


#if (NGX_HAVE_LOOKUP3)
static ngx_int_t
ngx_http_lookup3_hash_variable(ngx_http_request_t *r, ngx_http_variable_value_t *v,
    uintptr_t data)
{
    ngx_http_hash_ctx_t *ctx = (ngx_http_hash_ctx_t *)data;

    ngx_str_t                   val;

    if (ngx_http_complex_value(r, &ctx->value, &val) != NGX_OK) {
        return NGX_ERROR;
    }

    v->len = sizeof("3C32515A") - 1;

    if (v->len <= ctx->start) {
        return NGX_ERROR;
    }

    if (v->len < ctx->end) {
        return NGX_ERROR;
    }

    v->data = ngx_palloc(r->pool, v->len);
    if (v->data == NULL) {
        v->not_found = 1;
        return NGX_OK;
    }

    ngx_sprintf(v->data, "%08XD", ngx_lookup3_hashlittle(val.data, val.len, 0));

    v->valid = 1;
    v->not_found = 0;
    v->no_cacheable = 0;

    if (ctx->end) {
        v->data += ctx->start;
        v->len = ctx->end;
    }

    return NGX_OK;
}
#endif

static char *
ngx_http_set_generic_hash_by_slot(ngx_conf_t *cf, ngx_command_t *cmd, void *conf,
    ngx_http_get_variable_pt handler)
{
    u_char                            *p;

    ngx_str_t                         *value, name;
    ngx_http_variable_t               *var;
    ngx_http_hash_ctx_t               *ctx;
    ngx_http_compile_complex_value_t   ccv;

    value = cf->args->elts;

    ctx = ngx_palloc(cf->pool, sizeof(ngx_http_hash_ctx_t));
    if (ctx == NULL) {
        return NGX_CONF_ERROR;
    }

    ngx_memzero(&ccv, sizeof(ngx_http_compile_complex_value_t));

    if (cf->args->nelts == 4) {
        p = (u_char *) ngx_strchr(value[2].data, ':');
        if (p == NULL) {
            ngx_conf_log_error(NGX_LOG_EMERG, cf, 0,
                               "invalid substring \"%V\"", &value[2]);
            return NGX_CONF_ERROR;
        }

        ctx->start = ngx_atoi(value[2].data, p - value[2].data);
        if (ctx->start == NGX_ERROR) {
            ngx_conf_log_error(NGX_LOG_EMERG, cf, 0,
                               "invalid substring \"%V\"", &value[2]);
            return NGX_CONF_ERROR;
        }

        p++;                         /* skip ':' */

        ctx->end = ngx_atoi(p, value[2].data + value[2].len - p);
        if (ctx->end == NGX_ERROR) {
            ngx_conf_log_error(NGX_LOG_EMERG, cf, 0,
                               "invalid substring \"%V\"", &value[2]);
            return NGX_CONF_ERROR;
        }

        if (ctx->start >= ctx->end) {
            ngx_conf_log_error(NGX_LOG_EMERG, cf, 0,
                               "invalid substring \"%V\"", &value[2]);
            return NGX_CONF_ERROR;
        }

        ccv.value = &value[3];
    } else {
        ccv.value = &value[2];
    }

    ccv.cf = cf;
    ccv.complex_value = &ctx->value;

    if (ngx_http_compile_complex_value(&ccv) != NGX_OK) {
        return NGX_CONF_ERROR;
    }

    name = value[1];

    if (name.data[0] == '$') {
        name.len--;
        name.data++;
    }

    var = ngx_http_add_variable(cf, &name, NGX_HTTP_VAR_CHANGEABLE);
    if (var == NULL) {
        return NGX_CONF_ERROR;
    }    

    var->get_handler = handler;
    var->data = (uintptr_t) ctx;

    return NGX_CONF_OK;
}


static char *
ngx_http_set_crc32_hash_by_slot(ngx_conf_t *cf, ngx_command_t *cmd, void *conf)
{
    return ngx_http_set_generic_hash_by_slot(cf, cmd, conf, ngx_http_crc32_hash_variable);
}


static char *
ngx_http_set_md5_hash_by_slot(ngx_conf_t *cf, ngx_command_t *cmd, void *conf)
{
    return ngx_http_set_generic_hash_by_slot(cf, cmd, conf, ngx_http_md5_hash_variable);
}


#if (NGX_HAVE_LOOKUP3)
static char *
ngx_http_set_lookup3_hash_by_slot(ngx_conf_t *cf, ngx_command_t *cmd, void *conf)
{
    return ngx_http_set_generic_hash_by_slot(cf, cmd, conf, ngx_http_lookup3_hash_variable);
}
#endif
