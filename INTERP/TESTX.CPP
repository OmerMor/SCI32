

int OpenRobot(ResNum num)
{
	int	fd;
	char	nameBuf[MaxPath + 1];

	// check resource-config file path(s)...
	for (int i = 0; i < configMgr->GetNTokens("robot"); i++)  {
		MakeName(nameBuf, configMgr->Get("robot", i), num);
		msgMgr->Alert("%s", nameBuf);
		if ((fd = ::Open(nameBuf, O_BINARY | O_RDONLY)) != -1)
			return fd;
	}
	return -1;
}
