import { defineConfig } from 'vitepress'

const repositoryName = process.env.GITHUB_REPOSITORY?.split('/')[1]
const isGitHubActions = process.env.GITHUB_ACTIONS === 'true'
const isUserSite = repositoryName?.endsWith('.github.io')
const base = isGitHubActions && repositoryName && !isUserSite
  ? `/${repositoryName}/`
  : '/'

// https://vitepress.dev/reference/site-config
export default defineConfig({
  base,
  title: "CJIT",
  description: "C, Just in Time!",
  themeConfig: {
    // https://vitepress.dev/reference/default-theme-config
    nav: [
      { text: 'CJIT Tutorial', link: '/' },
      { text: 'About Dyne.org', link: 'https://dyne.org' }
    ],

    sidebar: [
      {
        text: 'Tutorial',
        items: [
          { text: 'Get started', link: '/tutorial' },
          { text: 'Graphics', link: '/graphics' },
          { text: 'Sound', link: '/sound' },
		  { text: 'Filesystem', link: '/filesystem' },
		  { text: 'Terminal UI', link: '/tui' },
		  { text: 'Manpage', link: '/manpage' },
		  { text: 'FAQs', link: '/faq' }
        ]
      }
    ],

    socialLinks: [
      { icon: 'github', link: 'https://github.com/dyne/cjit' }
    ]
  }
})
