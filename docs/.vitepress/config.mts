import { defineConfig } from 'vitepress'

const repositoryName = process.env.GITHUB_REPOSITORY?.split('/')[1]
const isGitHubActions = process.env.GITHUB_ACTIONS === 'true'
const isUserSite = repositoryName?.endsWith('.github.io')
const githubPagesBase = isGitHubActions && repositoryName && !isUserSite
  ? `/${repositoryName}/`
  : '/'
const base = process.env.BASE_PATH ?? githubPagesBase

export default defineConfig({
  title: 'CJIT',
  description: 'Run, compile, and link C programs instantly with CJIT.',
  base,
  appearance: true,
  head: [
    ['link', { rel: 'icon', href: `${base}favicon.png` }]
  ],
  themeConfig: {
    nav: [
      { text: 'Tutorial', link: '/tutorial' },
      { text: 'Manual', link: '/manpage' },
      { text: 'FAQ', link: '/faq' },
      { text: 'Releases', link: 'https://github.com/dyne/cjit/releases' },
      { text: 'About Dyne.org', link: 'https://dyne.org' }
    ],

    sidebar: [
      {
        text: 'Start',
        items: [
          { text: 'Overview', link: '/' },
          { text: 'Tutorial', link: '/tutorial' },
          { text: 'Command reference', link: '/manpage' },
          { text: 'Frequently asked questions', link: '/faq' }
        ]
      },
      {
        text: 'Cookbook',
        items: [
          { text: 'Graphics', link: '/graphics' },
          { text: 'Sound', link: '/sound' },
          { text: 'Filesystem', link: '/filesystem' },
          { text: 'Terminal UI', link: '/tui' }
        ]
      }
    ],

    socialLinks: [
      { icon: 'github', link: 'https://github.com/dyne/cjit' }
    ],

    editLink: {
      pattern: 'https://github.com/dyne/cjit/edit/main/docs/:path',
      text: 'Edit this page on GitHub'
    },

    search: {
      provider: 'local'
    },

    outline: {
      level: [2, 3]
    }
  }
})
