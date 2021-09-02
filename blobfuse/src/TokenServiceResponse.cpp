#include "TokenServiceResponse.h"
#include <iomanip>
#include <time.h>

void ts_data_to_json(json &j, const TokenServiceResponseData &t)
{
	j = json{
		{"access_token", t.token },
		{"expires_on", t.expireTime },
		{"token_type", t.type }
	};
}

void from_json(const json &j, TokenServiceResponseData &t)
{
	t.token = j.value("token", "");
	t.type = j.value("type", "");
	j.at("expireTime").get_to(t.expireTime);
}
