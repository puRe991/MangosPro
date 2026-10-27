-- MangosPro Support
-- The 2.4.3 Knowledge Base ("Customer Support" window) was loaded from Blizzard's servers and stays
-- empty on private servers. FrameXML only talks to it through the global KBSetup_* / KBQuery_* /
-- KBArticle_* / KBSystem_* functions, so this addon replaces them with a local article list
-- (Articles.lua) and fires the load events itself.

local categories = MangosProSupport_Categories;
local articles = MangosProSupport_Articles;

local setupList = {};
local queryList = {};
local current;

local function Fire(event)
	KnowledgeBaseFrame_OnEvent(KnowledgeBaseFrame, event);
end

-- one page of a result list
local function Page(list, perPage, page)
	local result = {};
	local first = (page - 1) * perPage + 1;
	for i = first, min(#list, first + perPage - 1) do
		tinsert(result, list[i]);
	end
	return result;
end

local function Header(list, index)
	local a = list[index];
	if ( not a ) then
		return;
	end
	return a.id, a.title, a.hot, a.new;
end

local function Matches(a, words)
	local haystack = strlower(a.title .. " " .. (a.keywords or "") .. " " .. a.text);
	for _, word in ipairs(words) do
		if ( not strfind(haystack, word, 1, true) ) then
			return false;
		end
	end
	return true;
end

-- Setup (top issues)
function KBSetup_BeginLoading(perPage, page)
	local all = {};
	for _, a in ipairs(articles) do
		if ( a.top ) then
			tinsert(all, a);
		end
	end
	setupList = Page(all, perPage, page);
	setupList.total = #all;
	Fire("KNOWLEDGE_BASE_SETUP_LOAD_SUCCESS");
end

function KBSetup_IsLoaded() return true; end
function KBSetup_GetCategoryCount() return #categories; end
function KBSetup_GetCategoryData(index) return index, categories[index]; end
function KBSetup_GetSubCategoryCount(category) return 0; end
function KBSetup_GetSubCategoryData(category, index) return nil; end
function KBSetup_GetLanguageCount() return 1; end
function KBSetup_GetLanguageData(index) return 0, "Deutsch"; end
function KBSetup_GetArticleHeaderCount() return #setupList; end
function KBSetup_GetTotalArticleCount() return setupList.total or 0; end
function KBSetup_GetArticleHeaderData(index) return Header(setupList, index); end

-- Search
function KBQuery_BeginLoading(searchText, category, subcategory, perPage, page)
	local words = {};
	for word in string.gmatch(strlower(searchText or ""), "%S+") do
		tinsert(words, word);
	end

	local all = {};
	for _, a in ipairs(articles) do
		if ( (category == 0 or a.category == category) and Matches(a, words) ) then
			tinsert(all, a);
		end
	end
	queryList = Page(all, perPage, page);
	queryList.total = #all;
	Fire("KNOWLEDGE_BASE_QUERY_LOAD_SUCCESS");
end

function KBQuery_IsLoaded() return true; end
function KBQuery_GetArticleHeaderCount() return #queryList; end
function KBQuery_GetTotalArticleCount() return queryList.total or 0; end
function KBQuery_GetArticleHeaderData(index) return Header(queryList, index); end

-- Article
function KBArticle_BeginLoading(id, searchType)
	current = nil;
	for _, a in ipairs(articles) do
		if ( a.id == id ) then
			current = a;
			break;
		end
	end
	if ( current ) then
		Fire("KNOWLEDGE_BASE_ARTICLE_LOAD_SUCCESS");
	else
		Fire("KNOWLEDGE_BASE_ARTICLE_LOAD_FAILURE");
	end
end

function KBArticle_IsLoaded() return current ~= nil; end
function KBArticle_GetData()
	if ( not current ) then
		return;
	end
	return current.id, current.title, current.title, current.text, current.keywords, 0, current.hot;
end

-- Banner lines above the search box
function KBSystem_GetMOTD() return MangosProSupport_Motd; end
function KBSystem_GetServerNotice() return MangosProSupport_ServerNotice; end
function KBSystem_GetServerStatus() return nil; end

-- The client's own loader still reports failures for the Blizzard service; ignore those.
local f = CreateFrame("Frame");
f:RegisterEvent("PLAYER_LOGIN");
f:SetScript("OnEvent", function()
	KnowledgeBaseFrame:UnregisterEvent("KNOWLEDGE_BASE_SETUP_LOAD_FAILURE");
	KnowledgeBaseFrame:UnregisterEvent("KNOWLEDGE_BASE_QUERY_LOAD_FAILURE");
	KnowledgeBaseFrame:UnregisterEvent("KNOWLEDGE_BASE_ARTICLE_LOAD_FAILURE");
	KnowledgeBaseFrame:UnregisterEvent("KNOWLEDGE_BASE_SYSTEM_MOTD_UPDATE");
	KnowledgeBaseFrame:UnregisterEvent("KNOWLEDGE_BASE_SERVER_MESSAGE");
	KBASE_ERROR_LOAD_FAILURE = "Der Hilfe-Artikel konnte nicht geladen werden. Über die Knöpfe unten kannst du ein GM-Ticket erstellen.";
end);
