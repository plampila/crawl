/**
 * @file
 * @brief Contains code for the `json-data` utility.
**/

#include "AppHdr.h"
#include "externs.h"
#include "artefact.h"
#include "art-enum.h"
#include "clua.h"
#include "coordit.h"
#include "database.h"
#include "directn.h"
#include "dungeon.h"
#include "env.h"
#include "initfile.h"
#include "itemname.h"
#include "itemprop.h"
#include "json.h"
#include "json-wrapper.h"
#include "shopping.h"
#include "spl-book.h"
#include "spl-util.h"
#include "spl-zap.h"
#include "state.h"
#include "stringutil.h"
#include "version.h"
#include "player.h"
#include <sstream>
#include <set>
#include <unistd.h>

// Clockwise, around the compass from north (same order as run_dir_type)
const struct coord_def Compass[9] =
{
    // kuln
    {0, -1}, {1, -1}, {1, 0}, {1, 1},
    // jbhy
    {0, 1}, {-1, 1}, {-1, 0}, {-1, -1},
    // .
    {0, 0}
};

static void _initialize_crawl()
{
    init_spell_descs();
    init_spell_name_cache();

    SysEnv.crawl_dir = "./";
    databaseSystemInit();
    init_spell_name_cache();
    init_spell_rarities();
}

static JsonNode *_spell_schools(spell_type spell)
{
    JsonNode *obj(json_mkobject());
    for (const auto bit : spschools_type::range())
    {
        if (spell_typematch(spell, bit))
        {
            json_append_member(obj, spelltype_long_name(bit),
                               json_mkbool(true));
        }
    }
    return obj;
}


static const map<spflag_type, string> _spell_flag_names =
{
    { SPFLAG_DIR_OR_TARGET, "DIR_OR_TARGET" },
    { SPFLAG_TARGET, "TARGET" },
    { SPFLAG_DIR, "DIR" },
    { SPFLAG_OBJ, "OBJ" },
    { SPFLAG_HELPFUL, "HELPFUL" },
    { SPFLAG_NEUTRAL, "NEUTRAL" },
    { SPFLAG_NOT_SELF, "NOT_SELF" },
    { SPFLAG_UNHOLY, "UNHOLY" },
    { SPFLAG_UNCLEAN, "UNCLEAN" },
    { SPFLAG_CHAOTIC, "CHAOTIC" },
    { SPFLAG_HASTY, "HASTY" },
    { SPFLAG_EMERGENCY, "EMERGENCY" },
    { SPFLAG_ESCAPE, "ESCAPE" },
    { SPFLAG_RECOVERY, "RECOVERY" },
    { SPFLAG_AREA, "AREA" },
    { SPFLAG_SELFENCH, "SELFENCH" },
    { SPFLAG_MONSTER, "MONSTER" },
    { SPFLAG_NEEDS_TRACER, "NEEDS_TRACER" },
    { SPFLAG_NOISY, "NOISY" },
    { SPFLAG_TESTING, "TESTING" },
    { SPFLAG_CORPSE_VIOLATING, "CORPSE_VIOLATING" },
    { SPFLAG_UTILITY, "UTILITY" },
    { SPFLAG_NO_GHOST, "NO_GHOST" },
    { SPFLAG_CLOUD, "CLOUD" },
    { SPFLAG_MR_CHECK, "MR_CHECK" },
    { SPFLAG_MONS_ABJURE, "MONS_ABJURE" },
    { SPFLAG_NOT_EVIL, "NOT_EVIL" },
    { SPFLAG_HOLY, "HOLY" },
};

static JsonNode *_spell_flags(spell_type spell)
{
    JsonNode *obj(json_mkobject());
    unsigned flags = get_spell_flags(spell);
    for (auto const &mapping : _spell_flag_names)
    {
        if (flags & mapping.first)
            json_append_member(obj, mapping.second.c_str(), json_mkbool(true));
    }
    return obj;
}

static const char *_book_name(book_type book)
{
    item_def item;
    item.base_type = OBJ_BOOKS;
    item.sub_type  = book;
    return item.name(DESC_PLAIN, false, true).c_str();
}

static JsonNode *_spell_books(spell_type spell)
{
    JsonNode *obj(json_mkobject());
    for (int i = 0; i < NUM_FIXED_BOOKS; ++i)
    {
        const book_type book = static_cast<book_type>(i);
        if (!item_type_removed(OBJ_BOOKS, book))
        {
            vector<spell_type> list = spellbook_template(book);
            if (std::find(list.begin(), list.end(), spell) != list.end())
                json_append_member(obj, _book_name(book), json_mkbool(true));
        }
    }
    return obj;
}


static JsonNode *_spell_range(spell_type spell)
{
    JsonNode *obj(json_mkobject());
    json_append_member(obj, "min", json_mknumber(spell_range(spell, 0)));
    json_append_member(obj, "max",
                       json_mknumber(spell_range(spell,
                                                 spell_power_cap(spell))));
    return obj;
}

static JsonNode *_spell_noise(spell_type spell)
{
    JsonNode *obj(json_mkobject());

    // from spell_noise_string
    int effect_noise = spell_effect_noise(spell);
    zap_type zap = spell_to_zap(spell);
    if (effect_noise == 0 && zap != NUM_ZAPS)
    {
        bolt beem;
        zappy(zap, 0, false, beem);
        effect_noise = beem.loudness;
    }
    if (spell == SPELL_TORNADO)
        effect_noise = 15;

    json_append_member(obj, "casting", json_mknumber(spell_noise(spell)));
    json_append_member(obj, "effect", json_mknumber(effect_noise));

    return obj;
}

static JsonNode *_spell_object(spell_type spell)
{
    JsonNode *obj(json_mkobject());

    string name = string(spell_title(spell));
    json_append_member(obj, "name", json_mkstring(name.c_str()));
    json_append_member(obj, "level", json_mknumber(spell_difficulty(spell)));
    json_append_member(obj, "schools", _spell_schools(spell));
    json_append_member(obj, "power cap",
                       json_mknumber(spell_power_cap(spell)));
    if (spell_range(spell, spell_power_cap(spell)) != -1)
        json_append_member(obj, "range", _spell_range(spell));
    json_append_member(obj, "noise", _spell_noise(spell));
    json_append_member(obj, "flags", _spell_flags(spell));
    json_append_member(obj, "rarity", json_mknumber(spell_rarity(spell)));
    json_append_member(obj, "description",
                       json_mkstring(trimmed_string(
                           getLongDescription(name + " spell")).c_str()));
    string quote = getQuoteString(name + " spell");
    if (!quote.empty())
    {
        json_append_member(obj, "quote",
                           json_mkstring(trimmed_string(quote).c_str()));
    }
    json_append_member(obj, "books", _spell_books(spell));

    return obj;
}

static JsonNode *_spell_list()
{
    JsonNode *obj(json_mkobject());
    for (int i = SPELL_NO_SPELL + 1; i < NUM_SPELLS; ++i)
    {
        const spell_type spell = static_cast<spell_type>(i);
        if (!is_valid_spell(spell) || !is_player_spell(spell))
            continue;

        json_append_member(obj, spell_title(spell), _spell_object(spell));
    }
    return obj;
}

static JsonNode *_book_spells(book_type book)
{
    JsonNode *array(json_mkarray());
    for (const spell_type spell : spellbook_template(book))
        json_append_element(array, json_mkstring(spell_title(spell)));
    return array;
}

static JsonNode *_book_object(book_type book)
{
    item_def item;
    item.base_type = OBJ_BOOKS;
    item.sub_type = book;
    item.quantity = 1;

    JsonNode *obj(json_mkobject());
    json_append_member(obj, "name", json_mkstring(_book_name(book)));
    json_append_member(obj, "spells", _book_spells(book));
    json_append_member(obj, "rarity", json_mknumber(book_rarity(book)));
    if (is_rare_book(book))
        json_append_member(obj, "rare", json_mkbool(true));
    json_append_member(obj, "value", json_mknumber(item_value(item, true)));
    json_append_member(obj, "description",
                       json_mkstring(trimmed_string(
                           getLongDescription(_book_name(book))).c_str()));
    string quote = getQuoteString(_book_name(book));
    if (!quote.empty())
    {
        json_append_member(obj, "quote",
                           json_mkstring(trimmed_string(quote).c_str()));
    }
    return obj;
}

static JsonNode *_book_list()
{
    JsonNode *obj(json_mkobject());
    for (int i = 0; i < NUM_FIXED_BOOKS; ++i)
    {
        const book_type book = static_cast<book_type>(i);
        if (!item_type_removed(OBJ_BOOKS, book))
            json_append_member(obj, _book_name(book), _book_object(book));
    }
    return obj;
}

static const map<unrand_flag_type, string> _unrand_flag_names = {
    { UNRAND_FLAG_SPECIAL, "SPECIAL" },
    { UNRAND_FLAG_HOLY, "HOLY" },
    { UNRAND_FLAG_EVIL, "EVIL" },
    { UNRAND_FLAG_UNCLEAN, "UNCLEAN" },
    { UNRAND_FLAG_CHAOTIC, "CHAOTIC" },
    { UNRAND_FLAG_CORPSE_VIOLATING, "CORPSE_VIOLATING" },
    { UNRAND_FLAG_NOGEN, "NOGEN" },
    { UNRAND_FLAG_RANDAPP, "RANDAPP" },
    { UNRAND_FLAG_UNIDED, "UNIDED" },
    { UNRAND_FLAG_SKIP_EGO, "SKIP_EGO" },
};

static JsonNode *_unrand_flags(const item_def &item)
{
    JsonNode *obj(json_mkobject());
    unsigned flags = get_unrand_entry(item.unrand_idx)->flags;
    for (auto const &mapping : _unrand_flag_names)
    {
        if (flags & mapping.first)
            json_append_member(obj, mapping.second.c_str(), json_mkbool(true));
    }
    return obj;
}

static JsonNode *_unrand_object(const item_def &item)
{
    const unrandart_entry* entry = get_unrand_entry(item.unrand_idx);

    JsonNode *obj(json_mkobject());
    json_append_member(obj, "name", json_mkstring(entry->name));
    json_append_member(obj, "full name",
                       json_mkstring(item.name(DESC_INVENTORY, false, true,
                                               true).c_str()));
    if (entry->unid_name != nullptr && entry->unid_name != entry->name)
    {
        json_append_member(obj, "name unidentified",
                           json_mkstring(entry->unid_name));
    }
    if (entry->type_name != nullptr)
        json_append_member(obj, "type name", json_mkstring(entry->type_name));
    if (entry->inscrip != nullptr)
        json_append_member(obj, "inscription", json_mkstring(entry->inscrip));
    json_append_member(obj, "base type",
            json_mkstring(base_type_string(item)));
    json_append_member(obj, "sub type",
            json_mkstring(sub_type_string(item, true).c_str()));
    json_append_member(obj, "value", json_mknumber(item_value(item, true)));
    json_append_member(obj, "flags", _unrand_flags(item));

    json_append_member(obj, "description",
                       json_mkstring(trimmed_string(
                               getLongDescription(entry->name)).c_str()));
    string quote = getQuoteString(entry->name);
    if (!quote.empty())
    {
        json_append_member(obj, "quote",
                           json_mkstring(trimmed_string(quote).c_str()));
    }
    return obj;
}

static JsonNode *_unrand_list()
{
    JsonNode *obj(json_mkobject());
    for (int i = 0; i < NUM_UNRANDARTS; ++i)
    {
        const int              index = i + UNRAND_START;
        const unrandart_entry* entry = get_unrand_entry(index);

        // Skip dummy entries.
        if (entry->base_type == OBJ_UNASSIGNED)
            continue;

        item_def item;
        make_item_unrandart(item, index);
        item.quantity = 1;
        set_ident_flags(item, ISFLAG_IDENT_MASK);

        json_append_member(obj, entry->name, _unrand_object(item));
    }
    return obj;
}

int main(int argc, char* argv[])
{
    alarm(5);
    crawl_state.test = true;

    _initialize_crawl();

    JsonWrapper json(json_mkobject());
    json_append_member(json.node, "version", json_mkstring(Version::Long));
    json_append_member(json.node, "spells", _spell_list());
    json_append_member(json.node, "spellbooks", _book_list());
    json_append_member(json.node, "unrands", _unrand_list());
    printf("%s\n", json.to_string().c_str());

    databaseSystemShutdown();

    return 0;
}

//////////////////////////////////////////////////////////////////////////
// main.cc stuff

CLua clua(true);
CLua dlua(false);      // Lua interpreter for the dungeon builder.
crawl_environment env; // Requires dlua.
player you;
game_state crawl_state;

void process_command(command_type);
void process_command(command_type) {}

void world_reacts();
void world_reacts() {}
