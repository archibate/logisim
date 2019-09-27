run: lgs test.lgs
	./$^
	@if [ -f ~/.eh ]; then ~/.eh false; fi
